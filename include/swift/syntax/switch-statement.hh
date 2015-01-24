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

#ifndef swift_syntax_switch_statement_hh
#define swift_syntax_switch_statement_hh

#include "swift/syntax/branch-statement.hh"
#include "swift/support/iterator-range.hh"

#include <vector>

namespace swift::ast {
class case_statement;
class context;
class expression;
class pattern;

class switch_statement : public branch_statement {
public:
  using case_item =
      std::tuple<std::vector<std::tuple<ast::pattern *, ast::expression *>>,
                 ast::statement *>;

private:
  ast::expression *control_expression_;
  std::vector<case_item> case_statements_;

public:
  void *operator new(size_t size, const ast::context &context,
                     unsigned alignment = 8);

  switch_statement(ast::expression *control_expression,
                   std::vector<case_item> &case_statements)
      : ast::branch_statement(branch_statement::type::switch_statement),
        control_expression_(control_expression),
        case_statements_(case_statements) {}

  const ast::expression *control_expression() const noexcept {
    return control_expression_;
  }
  iterator_range<std::vector<case_item>::const_iterator> cases() const
      noexcept {
    return { std::begin(case_statements_), std::end(case_statements_) };
  }

private:
  void *operator new(size_t) noexcept {
    swift_unreachable("statement cannot be allocated with 'new'");
  }
  void operator delete(void *) noexcept {
    swift_unreachable("statement cannot be unallocated with 'delete'");
  }
};
}

#endif

