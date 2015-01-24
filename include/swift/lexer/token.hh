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

#ifndef swift_lexer_token_hh
#define swift_lexer_token_hh

#include <ext/string_view>
#include <type_traits>
#include <ostream>

#include "swift/lexer/location.hh"

namespace swift {
class token {
public:
  enum class type {
#define TOKEN(token) token,
#include "swift/lexer/tokens.def"
#undef TOKEN
  };

  enum class literal_type {
    invalid,         // invalid
    integral,        // integer-literal
    floating_point,  // floating-point-literal
    string,          // string-literal
    constant_true,   // 'true'
    constant_false,  // 'false'
    constant_nil,    // 'nil'
  };

  enum class operator_type {
    invalid,
    unary_prefix,
    unary_postfix,
    binary,
  };

private:
  type type_;
  range location_;
  std::u32string_view value_;
  literal_type literal_type_;
  operator_type operator_type_;

public:
  static const char32_t *canonical_spelling(token::type token_type);

  token() : type_(token::type::eof), location_() {}

  constexpr explicit token(enum type type)
      : type_(type), location_(), literal_type_(literal_type::invalid),
        operator_type_(operator_type::invalid) {
#if __cplusplus >= 201402l
    assert(type == type::invalid && "expected invalid type");
#endif
  }

  // generic
  token(enum type type, std::u32string_view value, location begin, location end)
      : type_(type), location_(begin, end), value_(value),
        literal_type_(literal_type::invalid),
        operator_type_(operator_type::invalid) {
    assert(not (type_ == token::type::op) && "use the operator constructor");
    assert(not (type_ == token::type::question) &&
           "use the operator constructor");
  }

  // identifier
  token(std::u32string_view value, location begin, location end)
      : type_(token::type::identifier), location_(begin, end), value_(value),
        literal_type_(literal_type::invalid),
        operator_type_(operator_type::invalid) {}

  // literal
  token(enum literal_type literal_type, std::u32string_view value,
        location begin, location end)
      : type_(token::type::literal), location_(begin, end), value_(value),
        literal_type_(literal_type), operator_type_(operator_type::invalid) {}

  // operator
  token(enum operator_type type, std::u32string_view value, location begin,
        location end)
      : type_(token::type::op), location_(begin, end), value_(value),
        literal_type_(literal_type::invalid), operator_type_(type) {}

  token(enum type type, enum operator_type op_type, std::u32string_view value,
        location begin, location end)
      : type_(type), location_(begin, end), value_(value),
        literal_type_(literal_type::invalid), operator_type_(op_type) {}

  operator enum type() const {
    return type_;
  }

  explicit operator enum literal_type() const {
    assert(type_ == token::type::literal && "token is not a literal");
    return literal_type_;
  }

  explicit operator enum operator_type() const {
    assert((type_ == token::type::op or type_ == token::type::question or
            type_ == token::type::exclaim) &&
           "token is not an operator or '?'");
    return operator_type_;
  }

  const range& location() const {
    return location_;
  }
  std::u32string_view value() const {
    return value_;
  }

  template <type Type>
  constexpr bool is() const {
    return type_ == Type;
  }

  template <literal_type Type>
  constexpr bool is() const {
    return type_ == type::literal and literal_type_ == Type;
  }

  template <operator_type Type>
  constexpr bool is() const {
    return (type_ == type::op or type_ == type::question or
            type_ == type::exclaim) and
           operator_type_ == Type;
  }
};
}

std::ostream& operator<<(std::ostream&, const swift::token&);
std::ostream& operator<<(std::ostream&, const swift::token::type&);
std::ostream& operator<<(std::ostream&, const swift::token::operator_type&);

#endif

