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

#ifndef swift_syntax_operator_declaration_hh
#define swift_syntax_operator_declaration_hh

#include "swift/syntax/declaration.hh"

#include <ext/string_view>

namespace swift::ast {
class declaration_context;

class operator_declaration : public declaration {
public:
  enum class type {
    infix,
    prefix,
    postfix,
  };

  enum class associativity {
    none,
    left,
    right,
  };

private:
  type type_;
  std::u32string_view name_;
  uint8_t precedence_;
  associativity associativity_;

public:
  operator_declaration(ast::declaration_context *declaration_context,
                       operator_declaration::type type,
                       std::u32string_view name, uint8_t precedence,
                       operator_declaration::associativity associativity)
      : ast::declaration(declaration::type::operator_declaration,
                         declaration_context),
        type_(type), name_(name), precedence_(precedence),
        associativity_(associativity) {}

  const type &type() const {
    return type_;
  }
  std::u32string_view name() const noexcept {
    return name_;
  }
  uint8_t precedence() const {
    return precedence_;
  }
  operator_declaration::associativity associativity() const {
    return associativity_;
  }
};
}

#endif

