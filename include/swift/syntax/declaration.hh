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

#ifndef swift_syntax_declaration_hh
#define swift_syntax_declaration_hh

#include "swift/syntax/statement.hh"

#include <cstddef>

namespace swift::ast {
class context;
class declaration_context;
class source_file;

class declaration : public statement {
  friend class declaration_context;

  static declaration *
  from_declaration_context(const ast::declaration_context *);

public:
  enum class type {
    top_level_declaration,

    import_declaration,
    constant_declaration,
    variable_declaration,
    typealias_declaration,
    function_declaration,
    enum_declaration,
    struct_declaration,
    class_declaration,
    protocol_declaration,
    initializer_declaration,
    deinitializer_declaration,
    extension_declaration,
    subscript_declaration,
    operator_declaration,
  };

private:
  declaration_context *declaration_context_;
  type type_;
  declaration *next_;

protected:
  declaration(declaration::type type, ast::declaration_context *context)
      : statement(statement::type::declaration), declaration_context_(context),
        type_(type), next_(nullptr) {}

public:
  void *operator new(size_t size, const ast::context &context,
                     ast::declaration_context *parent, size_t extra = 0);

  declaration::type type() const {
    return type_;
  }

  declaration_context *declaration_context() {
    return declaration_context_;
  }

  ast::context &ast_context();
  ast::source_file *source_file();
};
}

#endif

