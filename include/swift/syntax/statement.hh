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

#ifndef swift_syntax_statement_hh
#define swift_syntax_statement_hh

#include "swift/support/error-handling.hh"

#include <cstdlib>

namespace swift::ast {
class statement : public ast::visitable<statement> {
public:
  enum class type {
    expression,
    declaration,
    loop_statement,
    branch_statement,
    labelled_statement,
    control_transfer_statement,
    defer_statement,
    do_statement,
    compiler_control_statement,
    statements,
  };

private:
  enum type type_;

protected:
  explicit statement(statement::type type) : type_(type) {}

public:
  virtual ~statement() = 0;

  enum type type() const {
    return type_;
  }

protected:
  void *operator new(size_t) noexcept {
    swift_unreachable("statement cannot be allocated with 'new'");
  }
  void operator delete(void *) noexcept {
    swift_unreachable("statement cannot be unallocated with 'delete'");
  }
};

template <enum statement::type Type>
bool isa(const statement &statement) {
  return statement.type() == Type;
}

template <enum statement::type Type>
bool isa(const statement *statement) {
  return statement and statement->type() == Type;
}
}

#endif

