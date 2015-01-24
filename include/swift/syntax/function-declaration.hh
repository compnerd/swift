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

#ifndef swift_syntax_function_hh
#define swift_syntax_function_hh

#include "swift/syntax/declaration.hh"
#include "swift/syntax/declaration-context.hh"

#include <ext/string_view>
#include <vector>

namespace swift::ast {
class pattern;
class type;

class function_declaration : public declaration, public declaration_context {
  std::u32string_view name_;
  std::vector<ast::pattern *> parameter_clauses_;
  ast::type *result_type_;
  ast::statement *body_;

public:
  function_declaration(ast::declaration_context *declaration_context,
                       std::u32string_view name,
                       const std::vector<ast::pattern *> &parameter_clauses,
                       ast::type *result_type, ast::statement *body)
      : ast::declaration(declaration::type::function_declaration,
                         declaration_context),
        ast::declaration_context(declaration_context::type::function_declaration),
        name_(name), parameter_clauses_(parameter_clauses),
        result_type_(result_type), body_(body) {}

  ast::declaration_context *declaration_context() {
    return static_cast<ast::declaration_context *>(this);
  }

  std::u32string_view name() const {
    return name_;
  }
  const std::vector<const ast::pattern *> parameter_clauses() const noexcept {
    return std::vector<const ast::pattern *>(parameter_clauses_.begin(),
                                             parameter_clauses_.end());
  }
  const ast::type *result_type() const {
    return result_type_;
  }
  const ast::statement *body() const {
    return body_;
  }
};
}

#endif

