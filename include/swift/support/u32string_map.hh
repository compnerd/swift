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

#ifndef swift_support_u32string_map_hh
#define swift_support_u32string_map_hh

#include <llvm/Support/AlignOf.h>
#include <llvm/Support/Allocator.h>

#include <algorithm>
#include <ext/string_view>
#include <utility>

namespace swift {
class u32string_map_entry_base {
  unsigned length_;
public:
  explicit u32string_map_entry_base(unsigned length) : length_(length) {}
  unsigned key_length() const { return length_; }
};

class u32string_map_base {
  void initialise(unsigned size);

protected:
  u32string_map_entry_base **table_;
  unsigned buckets_;
  unsigned items_;
  unsigned tombstones_;
  unsigned item_size_;

  explicit u32string_map_base(unsigned item_size)
      : table_(nullptr), buckets_(0), items_(0), tombstones_(0),
        item_size_(item_size) {}

  u32string_map_base(u32string_map_base &&rhs)
      : table_(rhs.table_), buckets_(rhs.buckets_), items_(rhs.items_),
        tombstones_(rhs.tombstones_), item_size_(rhs.item_size_) {
    rhs.table_ = nullptr;
    rhs.buckets_ = 0;
    rhs.items_ = 0;
    rhs.tombstones_ = 0;
    rhs.item_size_ = 0;
  }

  u32string_map_base(unsigned initial_size, unsigned item_size);

  unsigned rebalance(unsigned bucket = 0);

  unsigned lookup_bucket_for(std::u32string_view key);

  int find_key(std::u32string_view key) const;

  void remove_key(u32string_map_entry_base *value);

  u32string_map_entry_base *remove_key(std::u32string_view key);

public:
  static u32string_map_entry_base *tombstone() {
    return reinterpret_cast<u32string_map_entry_base *>(-1);
  }

  unsigned buckets() const {
    return buckets_;
  }
  unsigned items() const {
    return items_;
  }
  unsigned size() const {
    return items_;
  }

  bool empty() const {
    return items_ == 0;
  }

  void swap(u32string_map_base &rhs) {
    std::swap(table_, rhs.table_);
    std::swap(buckets_, rhs.buckets_);
    std::swap(items_, rhs.items_);
    std::swap(tombstones_, rhs.tombstones_);
    std::swap(item_size_, rhs.item_size_);
  }
};

template <typename ValueTy>
class u32string_map_entry : public u32string_map_entry_base {
public:
  ValueTy second;

  u32string_map_entry(const u32string_map_entry &) = delete;

  explicit u32string_map_entry(unsigned string_length)
      : u32string_map_entry_base(string_length), second() {}

  template <typename InitTy>
  u32string_map_entry(unsigned string_length, InitTy &&value)
      : u32string_map_entry_base(string_length),
        second(std::forward<InitTy>(value)) {}

  const char32_t *key_data() const {
    return reinterpret_cast<const char32_t *>(this + 1);
  }

  std::u32string_view key() const {
    return std::u32string_view(key_data(), key_length());
  }

  const ValueTy &value() const { return second; }
  ValueTy &value() { return second; }

  template <typename AllocatorTy, typename InitTy>
  static u32string_map_entry *
  create(std::u32string_view key, AllocatorTy &allocator, InitTy &&value) {
    unsigned key_length = key.length();
    unsigned key_size = (key_length + 1) * sizeof(char32_t);

    unsigned size =
        static_cast<unsigned>(sizeof(u32string_map_entry)) + key_size;
    unsigned alignment = llvm::AlignOf<u32string_map_entry>::Alignment;

    u32string_map_entry *entry =
        static_cast<u32string_map_entry *>(allocator.Allocate(size, alignment));
    new (entry) u32string_map_entry(key_size, std::forward<InitTy>(value));

    char32_t *buffer = const_cast<char32_t *>(entry->key_data());
    std::memcpy(buffer, key.data(), key_length * sizeof(char32_t));
    buffer[key.length()] = U'\0';

    return entry;
  }

  template <typename AllocatorTy>
  static u32string_map_entry *
  create(std::u32string_view key, AllocatorTy &allocator) {
    return create(key, allocator, ValueTy());
  }

  template <typename InitTy>
  static u32string_map_entry *create(std::u32string_view key, InitTy &&value) {
    llvm::MallocAllocator allocator;
    return create(key, allocator, std::forward<InitTy>(value));
  }

  static u32string_map_entry *create(std::u32string_view key) {
    return create(key, ValueTy());
  }

  template <typename AllocatorTy>
  void destroy(AllocatorTy &allocator) {
    unsigned key_size = (key_length() + 1) * sizeof(char32_t);
    unsigned size =
        static_cast<unsigned>(sizeof(u32string_map_entry)) + key_size;
    this->~u32string_map_entry();
    allocator.Deallocate(static_cast<void *>(this), size);
  }

  void destroy() {
    llvm::MallocAllocator allocator;
    destroy(allocator);
  }

  static const u32string_map_entry &from_key_data(const char32_t *key_data) {
    const uint8_t *pointer = reinterpret_cast<const uint8_t *>(key_data) -
                             sizeof(u32string_map_entry<ValueTy>);
    return *reinterpret_cast<const u32string_map_entry *>(pointer);
  }
};

template <typename ValueTy>
class u32string_map_const_iterator {
protected:
  u32string_map_entry_base **pointer_;

public:
  typedef u32string_map_entry<ValueTy> value_type;

  u32string_map_const_iterator() : pointer_(nullptr) {}

  explicit u32string_map_const_iterator(u32string_map_entry_base **bucket,
                                        bool advance = true)
      : pointer_(bucket) {
    if (advance)
      advance_past_empty_buckets();
  }

  const value_type &operator*() const {
    return *static_cast<u32string_map_entry<ValueTy> *>(*pointer_);
  }
  const value_type *operator->() const {
    return static_cast<u32string_map_entry<ValueTy> *>(*pointer_);
  }

  bool operator==(const u32string_map_const_iterator &rhs) const {
    return pointer_ == rhs.pointer_;
  }
  bool operator!=(const u32string_map_const_iterator &rhs) const {
    return not operator==(rhs);
  }

  inline u32string_map_const_iterator &operator++() {
    ++pointer_;
    advance_past_empty_buckets();
    return *this;
  }
  inline u32string_map_const_iterator &operator++(int) {
    u32string_map_const_iterator previous = *this;
    ++this;
    return previous;
  }

private:
  void advance_past_empty_buckets() {
    while (*pointer_ == nullptr or *pointer_ == u32string_map_base::tombstone())
      ++pointer_;
  }
};

template <typename ValueTy>
class u32string_map_iterator : public u32string_map_const_iterator<ValueTy> {
public:
  u32string_map_iterator() {}

  explicit u32string_map_iterator(u32string_map_entry_base **bucket,
                                  bool advance = true)
      : u32string_map_const_iterator<ValueTy>(bucket, advance) {}
  u32string_map_entry<ValueTy> &operator*() const {
    return *static_cast<u32string_map_entry<ValueTy> *>(*this->pointer_);
  }
  u32string_map_entry<ValueTy> *operator->() const {
    return static_cast<u32string_map_entry<ValueTy> *>(*this->pointer_);
  }
};

template <typename ValueTy, typename AllocatorTy = llvm::MallocAllocator>
class u32string_map : public u32string_map_base {
  AllocatorTy allocator_;

public:
  typedef const char32_t *key_type;
  typedef u32string_map_entry<ValueTy> value_type;
  typedef size_t size_type;

  typedef u32string_map_iterator<ValueTy> iterator;
  typedef u32string_map_const_iterator<ValueTy> const_iterator;

  u32string_map()
      : u32string_map_base(static_cast<unsigned>(sizeof(value_type))) {}

  explicit u32string_map(unsigned initial_size)
      : u32string_map_base(initial_size,
                           static_cast<unsigned>(sizeof(value_type))) {}

  explicit u32string_map(AllocatorTy allocator)
      : u32string_map_base(static_cast<unsigned>(sizeof(value_type))),
        allocator_(allocator) {}

  u32string_map(unsigned initial_size, AllocatorTy allocator)
      : u32string_map_base(initial_size,
                           static_cast<unsigned>(sizeof(value_type))),
        allocator_(allocator) {}

  u32string_map(u32string_map &&rhs)
      : u32string_map_base(std::move(rhs)),
        allocator_(std::move(rhs.allocator_)) {}

  u32string_map &operator=(u32string_map &rhs) {
    u32string_map_base::swap(rhs);
    std::swap(allocator_, rhs.allocator_);
    return *this;
  }

  ~u32string_map() {
    if (not empty()) {
      for (unsigned bucket = 0; not(bucket == buckets_); ++bucket) {
        u32string_map_entry_base *entry = table_[bucket];
        if (entry and not(entry == tombstone()))
          static_cast<value_type *>(entry)->destroy(allocator_);
      }
    }
    free(table_);
  }

  // TODO(compnerd) implement copy operators if/when they are needed

  AllocatorTy &allocator() {
    return allocator_;
  }
  const AllocatorTy &allocator() const {
    return allocator_;
  }

  iterator begin() {
    return iterator(table_, buckets_);
  }
  iterator end() {
    return iterator(table_, false);
  }

  const_iterator begin() const {
    return const_iterator(table_, buckets_);
  }
  const_iterator end() const {
    return const_iterator(table_, false);
  }

  iterator find(std::u32string_view key) {
    int bucket = find_key(key);
    if (bucket == -1)
      return end();
    return iterator(table_ + bucket, false);
  }
  const_iterator find(std::u32string_view key) const {
    int bucket = find_key(key);
    if (bucket == -1)
      return end();
    return const_iterator(table_ + bucket, false);
  }

  value_type lookup(std::u32string_view key) const {
    const_iterator item = find(key);
    if (item == end())
      return ValueTy();
    return item->second;
  }

  ValueTy &operator[](std::u32string_view key) {
    return insert(std::make_pair(key, ValueTy())).first->second;
  }

  bool insert(value_type *key_value) {
    unsigned bucket_id = lookup_bucket_for(key_value->key());
    u32string_map_entry_base *&bucket = table_[bucket_id];
    if (bucket and not(bucket == tombstone()))
      return false;

    if (bucket == tombstone())
      --tombstones_;
    bucket = key_value;
    ++items_;
    assert(items_ + tombstones_ <= buckets_);

    rebalance();
    return true;
  }

  std::pair<iterator, bool> insert(std::pair<std::u32string_view, ValueTy> kv) {
    unsigned bucket_id = lookup_bucket_for(kv.first);

    u32string_map_entry_base *&bucket = table_[bucket_id];
    if (bucket and not (bucket == tombstone()))
      return std::make_pair(iterator(table_ + bucket_id, true), false);

    if (bucket == tombstone())
      --tombstones_;
    bucket = value_type::create(kv.first, allocator_, std::move(kv.second));

    ++items_;
    assert(items_ + tombstones_ <= buckets_);
    bucket_id = rebalance(bucket_id);

    return std::make_pair(iterator(table_ + bucket_id, true), false);
  }

  void remove(value_type *key_value) {
    remove_key(key_value);
  }

  void erase(iterator entry) {
    value_type &value = *entry;
    remove(&value);
    value.destroy(allocator_);
  }

  bool erase(std::u32string_view key) {
    iterator entry = find(key);
    if (entry == end())
      return false;
    erase(entry);
    return true;
  }
};
}

#endif

