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

#ifndef swift_syntax_declaration_group_hh
#define swift_syntax_declaration_group_hh

namespace swift {
namespace ast {
class declaration;

class declaration_group {
  unsigned count_;

  declaration_group() : count_(0) {}
  declaration_group(unsigned count, declaration**);

public:
  static declaration_group*
  create(context& context, declaration** declarations, unsigned count);

  unsigned size() const {
    return count_;
  }

  declaration*& operator[](unsigned index) {
    assert(index < count_ && "out-of-bounds access");
    return reinterpret_cast<declaration**>(this + 1)[index];
  }

  declaration* const& operator[](unsigned index) const {
    assert(index < count_ && "out-of-bounds access");
    return reinterpret_cast<declaration* const*>(this + 1)[index];
  }
};

class declaration_group_reference {
  declaration* declaration_;
  enum class type {
    single_declaration = 0x0,
    declaration_group = 0x1,
    mask = 0x1
  };

  declaration_group_reference::type type() const {
    auto mask = reinterpret_cast<uintptr_t>(declaration_) |
                static_cast<uintptr_t>(type::mask);
    return static_cast<enum declaration_group_reference::type>(mask);
  }

public:
  typedef declaration** iterator;
  typedef declaration* const* const_iterator;

  declaration_group_reference() : declaration_(nullptr) {}

  explicit declaration_group_reference(declaration* decl)
    : declaration_(decl) {
    assert((~reinterpret_cast<uintptr_t>(decl) & static_cast<uintptr_t>(type::mask)) ==
               static_cast<uintptr_t>(type::mask) &&
           "expected byte-aligned value");
  }

  explicit declaration_group_reference(declaration_group* group)
    : declaration_(reinterpret_cast<ast::declaration*>(reinterpret_cast<uintptr_t>(group) | static_cast<uintptr_t>(type::declaration_group))) {
    assert((~reinterpret_cast<uintptr_t>(group) & static_cast<uintptr_t>(type::mask)) ==
               static_cast<uintptr_t>(type::mask) &&
           "expected byte-aligned value");
  }

  static declaration_group_reference
  create(ast::context& context, declaration** declarations, unsigned count) {
    if (count == 0)
      return declaration_group_reference();
    if (count == 1)
      return declaration_group_reference(declarations[0]);

    auto group = declaration_group::create(context, declarations, count);
    return declaration_group_reference(group);
  }

  bool is_null() const {
    return declaration_ == nullptr;
  }
  bool is_single_declaration() const {
    return type() == type::single_declaration;
  }
  bool is_declaration_group() const {
    return type() == type::declaration_group;
  }

  ast::declaration* declaration() {
    assert(is_single_declaration() && "not a single declaration");
    return declaration_;
  }

  ast::declaration_group& declaration_group() {
    assert(is_declaration_group() && "not a declaration group");
    auto pointer = reinterpret_cast<uintptr_t>(declaration_) & ~static_cast<uintptr_t>(type::mask);
    return *reinterpret_cast<ast::declaration_group*>(pointer);
  }
  const ast::declaration_group& declaration_group() const {
    return const_cast<declaration_group_reference*>(this)->declaration_group();
  }

  iterator begin() {
    if (is_single_declaration())
      return declaration_ ? &declaration_ : nullptr;
    return &declaration_group()[0];
  }
  const_iterator begin() const {
    if (is_single_declaration())
      return declaration_ ? &declaration_ + 1 : nullptr;
    return &declaration_group()[0];
  }
  iterator end() {
    if (is_single_declaration())
      return declaration_ ? &declaration_ + 1 : nullptr;
    ast::declaration_group& group = declaration_group();
    return &group[group.size()];
  }
  const_iterator end() const {
    if (is_single_declaration())
      return declaration_ ? &declaration_ + 1 : nullptr;
    const ast::declaration_group& group = declaration_group();
    return &group[group.size()];
  }
};
}
}

#endif

