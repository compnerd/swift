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

#ifndef swift_syntax_expression_hh
#define swift_syntax_expression_hh

#include "swift/syntax/statement.hh"

#include <llvm/ADT/APSInt.h>

namespace swift {
namespace ast {
class context;

class expression : public statement {
public:
  enum class type {
    /* prefix expression */
    prefix_unary_expression,
    in_out_expression,

    /* binary expression */
    sequence_expression,
    assignment_expression,
    conditional_expression,
    type_casting_expression,

    /* primary expressions */
    declaration_reference_expression,
    literal_expression,
    // self_expression,
    superclass_expression,
    closure_expression,
    parenthesized_expression,
    implicit_member_expression,
    wildcard_expression,

    /* postfix expression */
    // primary-expression
    postfix_unary_expression,
    function_call_expression,
    initializer_expression,
    explicit_member_expression,
    postfix_self_expression,
    dynamic_type_expression,
    subscript_expression,
    forced_value_expression,
    optional_chaining_expression,
  };

private:
  enum type type_;

public:
  void *operator new(size_t size, const ast::context &context,
                     unsigned alignment = 8);

  expression(expression::type type)
      : statement(statement::type::expression), type_(type) {}

  expression::type type() const {
    return type_;
  }

protected:
  void *operator new(size_t) noexcept {
    swift_unreachable("expression cannot be allocated with 'new'");
  }
  void operator delete(void *) noexcept {
    swift_unreachable("expression cannot be unallocated with 'delete'");
  }
};
}
}

#endif

