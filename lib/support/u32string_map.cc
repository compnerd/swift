/**
 * Copyright © 2014 Saleem Abdulrasool <compnerd@compnerd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include "swift/support/u32string_map.hh"
#include "swift/support/ucs4-support.hh"

namespace swift {
void u32string_map_base::initialise(unsigned size) {
  assert((size & (size - 1)) == 0 && "initial size must be power-of-2 or zero");
  buckets_ = size ? size : 16;
  items_ = 0;
  tombstones_ = 0;

  // allocate an extra bucket, making it appear filled to stop the iterator
  table_ = reinterpret_cast<u32string_map_entry_base **>(
      calloc(buckets_ + 1,
             sizeof(u32string_map_entry_base **) + sizeof(unsigned)));
  table_[buckets_] = reinterpret_cast<u32string_map_entry_base *>(2);
}

u32string_map_base::u32string_map_base(unsigned initial_size,
                                       unsigned item_size)
    : table_(nullptr), buckets_(0), items_(0), tombstones_(0),
      item_size_(item_size) {
  if (initial_size)
    initialise(initial_size);
}

unsigned u32string_map_base::rebalance(unsigned bucket) {
  unsigned new_size;
  unsigned *hashtable = reinterpret_cast<unsigned *>(table_ + buckets_ + 1);

  // rebalance if more than 75% full or less than 12.5% of the buckets are empty
  if (items_ * 4 > buckets_ * 3)
    new_size = buckets_ * 2;
  else if (buckets_ - (items_ + tombstones_) <= buckets_ / 8)
    new_size = buckets_;
  else
    return bucket;

  unsigned new_bucket = bucket;
  u32string_map_entry_base **new_table =
      reinterpret_cast<u32string_map_entry_base **>(
          calloc(new_size + 1,
                 sizeof(u32string_map_entry_base *) + sizeof(unsigned)));
  unsigned *new_hashtable =
      reinterpret_cast<unsigned *>(new_table + new_size + 1);
  new_table[new_size] = reinterpret_cast<u32string_map_entry_base *>(2);

  for (unsigned i = 0; i < buckets_; ++i) {
    u32string_map_entry_base *entry = table_[i];
    if (entry == nullptr or entry == tombstone())
      continue;

    unsigned hash_value = hashtable[i];
    unsigned new_bucket_id = hash_value & (new_size - 1);
    unsigned probe_depth = 1;
    while (new_table[new_bucket_id])
      new_bucket_id = (new_bucket_id + probe_depth++) & (new_size - 1);

    new_table[new_bucket_id] = entry;
    new_hashtable[new_bucket_id] = hash_value;
    if (i == bucket)
      new_bucket = new_bucket_id;
  }

  free(table_);

  table_ = new_table;
  buckets_ = new_size;
  tombstones_ = 0;

  return new_bucket;
}

unsigned u32string_map_base::lookup_bucket_for(std::u32string_view key) {
  if (buckets_ == 0)
    initialise(16);

  unsigned hash_value = hash(key);
  unsigned bucket = hash_value & (buckets_ - 1);
  unsigned *hashtable = reinterpret_cast<unsigned *>(table_ + buckets_ + 1);
  unsigned probe_depth = 1;
  int tombstoned = -1;

  while (true) {
    u32string_map_entry_base *entry = table_[bucket];
    if (entry == nullptr) {
      // If we found a tombstone, reuse the position instead of an empty bucket.
      // This reduces probing.
      hashtable[tombstoned == -1 ? bucket : tombstoned] = hash_value;
      return tombstoned == -1 ? bucket : tombstoned;
    }

    if (entry == tombstone()) {
      if (tombstoned == -1)
        tombstoned = bucket;
    } else if (hashtable[bucket] == hash_value) {
      // If the hash matches, do a deeper check to ensure that the value
      // matches.  The common case only looks at the buckets, not the items.
      // This is important for cache locality.

      // The entry may not be null-terminated, so check the key carefully
      auto *entry_key =
          reinterpret_cast<const std::u32string_view::value_type *>(entry + item_size_);
      if (key == std::u32string_view(entry_key, entry->key_length()))
        return bucket;
    }

    bucket = (bucket + probe_depth) & (buckets_ - 1);

    // use quadratic probing, it has fewer clumping artifacts than linear
    // probing, and good cache behaviour in the common case
    ++probe_depth;
  }
}

int u32string_map_base::find_key(std::u32string_view key) const {
  unsigned size = buckets_;
  if (size == 0)
    return -1;

  unsigned hash_value = hash(key);
  unsigned bucket = hash_value & (size - 1);
  unsigned *table = reinterpret_cast<unsigned *>(table_ + buckets_ + 1);
  unsigned probe_depth = 1;

  while (true) {
    u32string_map_entry_base *entry = table_[bucket];
    if (entry == nullptr)
      return -1;

    if (entry == tombstone()) {
      // ignore
    } else if (table[bucket] == hash_value) {
      // If the hash matches, do a deeper check to ensure that the value
      // matches.  The common case only looks at the buckets, not the items.
      // This is important for cache locality.

      // The entry may not be null-terminated, so check the key carefully
      auto *entry_key =
          reinterpret_cast<const std::u32string_view::value_type *>(entry + item_size_);
      if (key == std::u32string_view(entry_key, entry->key_length()))
        return bucket;
    }

    bucket = (bucket + probe_depth) & (size - 1);

    // use quadratic probing, it has fewer clumping artifacts than linear
    // probing, and good cache behaviour in the common case
    ++probe_depth;
  }
}

void u32string_map_base::remove_key(u32string_map_entry_base *value) {
  auto *entry =
      reinterpret_cast<const std::u32string_view::value_type *>(value + item_size_);
  u32string_map_entry_base *key =
      remove_key(std::u32string_view(entry, value->key_length()));
  assert(key && "key to be removed was not found in the table");
  (void)key; // silence warning
}

u32string_map_entry_base *
u32string_map_base::remove_key(std::u32string_view key) {
  int bucket = find_key(key);
  if (bucket == -1)
    return nullptr;

  u32string_map_entry_base *result = table_[bucket];

  table_[bucket] = tombstone();
  --items_;
  ++tombstones_;
  assert(items_ + tombstones_ <= buckets_);

  return result;
}
}

