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

#ifndef swift_semantics_identifier_hh
#define swift_semantics_identifier_hh

#include "swift/lexer/token.hh"
#include "swift/support/u32string_map.hh"

#include <ext/string_view>

namespace swift {
class identifier_info {
  friend class identifier_table;

  token::type token_type_;
  u32string_map_entry<identifier_info *> *entry_;

  static_assert(sizeof(token_type_) == sizeof(char32_t),
                "token::type not the same size as char32_t");

public:
  identifier_info(const identifier_info &) = delete;
  identifier_info &operator=(const identifier_info &) = delete;

  identifier_info(token::type token_type) : token_type_(token_type) {}

  const char32_t *name() const {
    assert(entry_ && "expected entry to set");
    return entry_->key_data();
  }

  token::type token_type() const {
    return token_type_;
  }
};

class identifier_table {
  u32string_map<identifier_info *, llvm::BumpPtrAllocator> hashtable_;

  void initialise();

public:
  typedef u32string_map<identifier_info *,
                        llvm::BumpPtrAllocator>::const_iterator iterator;
  typedef u32string_map<identifier_info *,
                        llvm::BumpPtrAllocator>::const_iterator const_iterator;

  identifier_table();

  identifier_info &get(std::u32string_view name, token::type token_type) {
    auto &entry = *hashtable_.insert(std::make_pair(name, nullptr)).first;
    if (identifier_info *&info = entry.second)
      return *info;
    (void)name.length();

    void *memory = hashtable_.allocator().Allocate<identifier_info>();
    new (memory) identifier_info(token_type);
    reinterpret_cast<identifier_info *>(memory)->entry_ = &entry;
    return *reinterpret_cast<identifier_info *>(memory);
  }
  identifier_info &get(std::u32string_view name) {
    return get(name, token::type::identifier);
  }

  iterator begin() const {
    return hashtable_.begin();
  }
  iterator end() const {
    return hashtable_.end();
  }
};
}

#endif

