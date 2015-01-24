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

#ifndef swift_syntax_magic_literal_expression_hh
#define swift_syntax_magic_literal_expression_hh

#include "swift/syntax/literal-expression.hh"

#include <ext/string_view>

namespace swift::ast {
class magic_literal_expression : public literal_expression {
public:
  enum class type {
    column,
    file,
    function,
    line,
  };

private:
  magic_literal_expression::type type_;
  std::u32string_view string_value_;
  uintmax_t integer_value_;

public:
  magic_literal_expression(magic_literal_expression::type type,
                           std::u32string_view value)
      : literal_expression(literal_expression::type::magic_literal),
        type_(type), string_value_(value) {
    assert((type == magic_literal_expression::type::file or
            type == magic_literal_expression::type::function) &&
           "expected file or function type");
  }

  magic_literal_expression(magic_literal_expression::type type, uintmax_t value)
      : literal_expression(literal_expression::type::magic_literal),
        type_(type), integer_value_(value) {
    assert((type == magic_literal_expression::type::column or
            type == magic_literal_expression::type::line) &&
           "expected column or line");
  }

  magic_literal_expression::type type() const {
    return type_;
  }
  std::u32string_view string_value() const {
    assert((type_ == magic_literal_expression::type::file or
            type_ == magic_literal_expression::type::function) &&
           "expected file or function magic literal");
    return string_value_;
  }
  uintmax_t integer_value() const {
    assert((type_ == magic_literal_expression::type::column or
            type_ == magic_literal_expression::type::line) &&
           "expected column or line magic literal");
    return integer_value_;
  }
};
}

#endif

