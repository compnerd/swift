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

#ifndef swift_semantic_scope_hh
#define swift_semantic_scope_hh

namespace swift {
namespace semantic {
class scope {
public:
  enum class type {
    top_level,               // top-level-declaration
    declaration,             // declaration
    enumeration,             // enum-statement
    expression,              // expression
    branch,                  // branch-statement
    function,                // function
    statement,               // statement
    switch_statement,        // switch-statement
    switch_case,             // case-statement
    compiler_configuration,  // if-configuration

    pattern,
  };

private:
  scope *parent_;
  type type_;

public:
  scope(scope *parent, enum type type) : parent_(parent), type_(type) {}

  enum type type() const noexcept {
    return type_;
  }
  void type(enum type value) noexcept {
    type_ = value;
  }

  const scope *parent() const noexcept {
    return parent_;
  }
  void parent(scope *value) noexcept {
    parent_ = value;
  }

  template <enum scope::type Type>
  bool is() const noexcept {
    return type_ == Type;
  }
};

class scope_raii {
  scope &scope_;
  enum scope::type parent_type_;
  bool active_;

public:
  scope_raii(scope &scope, enum scope::type current_type)
      : scope_(scope), parent_type_(scope.type()), active_(true) {
    scope.type(current_type);
  }

  ~scope_raii() {
    reset();
  }

  void reset() {
    if (active_)
      scope_.type(parent_type_);
    active_ = false;
  }
};
}
}

#endif

