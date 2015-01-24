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

#ifndef swift_lexer_lexer_hh
#define swift_lexer_lexer_hh

#include "swift/diagnostics/engine.hh"
#include "swift/lexer/identifier-table.hh"
#include "swift/lexer/location.hh"
#include "swift/lexer/token.hh"

#include <deque>

namespace swift {
class lexer {
  diagnostics::engine &diagnostics_engine_;

  const char32_t *buffer_start_;
  const char32_t *buffer_end_;
  const char32_t *cursor_;

  unsigned line_, column_;

  std::deque<token> lookahead_;
  identifier_table identifiers_;

  template <token::type Type>
  token consume();

  template <token::literal_type Type>
  token consume();

  template <token::type Type>
  bool match() const;

  location position() const {
    return { line_, column_ };
  }

  token lex();

public:
  lexer(diagnostics::engine &engine, const char32_t *buffer, size_t length)
      : diagnostics_engine_(engine), buffer_start_(buffer),
        buffer_end_(buffer + length), cursor_(buffer_start_), line_(1),
        column_(0) {}

  token head();
  token peek();
  token next();

  void set_buffer(const char32_t *buffer, size_t length);
};
}

#endif

