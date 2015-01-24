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

#include "swift/lexer/token.hh"

#include <codecvt>

std::ostream &operator<<(std::ostream &os, const swift::token &token) {
  if (token.is<swift::token::type::eof>()) {
    os << "{ <eof> }";
  } else {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf8;
    std::string lexeme = utf8.to_bytes(token.value().data());
    os << "{ lexeme:" << lexeme << "}";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const swift::token::type &) {
  // FIXME(compnerd) actually print the lexeme type
  return os;
}

std::ostream &
operator<<(std::ostream &os, const swift::token::operator_type &operator_type) {
  using namespace swift;
  const char *operator_type_name[] = {
    [static_cast<int>(token::operator_type::invalid)] = "invalid",
    [static_cast<int>(token::operator_type::binary)] = "binary",
    [static_cast<int>(token::operator_type::unary_prefix)] = "prefix unary",
    [static_cast<int>(token::operator_type::unary_postfix)] = "postfix unary",
  };

  os << operator_type_name[static_cast<int>(operator_type)];
  return os;
}

