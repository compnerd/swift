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

#ifndef swift_syntax_declaration_context_hh
#define swift_syntax_declaration_context_hh

#include "swift/syntax/declaration.hh"

namespace swift {
namespace ast {
class declaration_context {
public:
  enum class type {
    source_file,
    top_level_declaration,

    class_declaration,
    initializer_declaration,
    deinitializer_declaration,
    enum_declaration,
    extension_declaration,
    function_declaration,
    protocol_declaration,
    struct_declaration,
    subscript_declaration,
  };

private:
  declaration_context::type type_;

protected:
  mutable declaration *first_declaration_;
  mutable declaration *last_declaration_;

  declaration_context(declaration_context::type type)
      : type_(type), first_declaration_(nullptr), last_declaration_(nullptr) {}

public:
  virtual ~declaration_context();

  declaration_context::type type() const noexcept {
    return type_;
  }

  ast::declaration *declaration() {
    return declaration::from_declaration_context(this);
  }

  declaration_context *parent() {
    return declaration()->declaration_context();
  }

  bool is_source_file() const noexcept {
    return type_ == declaration_context::type::source_file;
  }

  ast::context &ast_context();
  void add_declaration(ast::declaration *declaration);

  const ast::declaration **begin() const noexcept {
    return const_cast<const ast::declaration **>(&first_declaration_);
  }
  const ast::declaration **end() const noexcept {
    return const_cast<const ast::declaration **>(&last_declaration_);
  }
};
}
}

#endif

