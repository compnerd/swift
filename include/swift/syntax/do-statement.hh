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

#ifndef swift_syntax_do_statement_hh
#define swift_syntax_do_statement_hh

#include "swift/syntax/statement.hh"
#include "swift/support/iterator-range.hh"

#include <tuple>
#include <vector>

namespace swift::ast {
class context;
class pattern;

class do_statement : public ast::statement {
  using catch_clause = std::tuple<ast::pattern *, ast::statement *>;

  ast::statement *body_;
  std::vector<catch_clause> catch_clauses_;

public:

  do_statement(ast::statement *body,
               std::vector<catch_clause> &catch_clauses)
      : ast::statement(statement::type::do_statement), body_(body),
        catch_clauses_(std::move(catch_clauses)) {}

  const ast::statement *body() const {
    return body_;
  }
  iterator_range<std::vector<catch_clause>::const_iterator>
  catch_clauses() const noexcept {
    return { std::begin(catch_clauses_), std::end(catch_clauses_) };
  }
};
}

#endif

