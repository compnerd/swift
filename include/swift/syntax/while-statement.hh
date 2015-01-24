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

#ifndef swift_syntax_while_statement_hh
#define swift_syntax_while_statement_hh

#include "swift/syntax/loop-statement.hh"
#include "swift/support/error-handling.hh"

namespace swift::ast {
class context;

class while_statement : public loop_statement {
  ast::statement *condition_;
  ast::statement *body_;

public:
  void *operator new(size_t size, const ast::context &context,
                     unsigned alignment = 8);

  while_statement(ast::statement *condition, ast::statement *body)
      : ast::loop_statement(loop_statement::type::while_statement),
        condition_(condition), body_(body) {}

  const ast::statement *condition() const noexcept {
    return condition_;
  }
  const ast::statement *body() const noexcept {
    return body_;
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

