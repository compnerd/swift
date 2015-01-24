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

#include "swift/syntax/declaration.hh"
#include "swift/syntax/context.hh"
#include "swift/syntax/class-declaration.hh"
#include "swift/syntax/declaration-context.hh"
#include "swift/syntax/enum-declaration.hh"
#include "swift/syntax/extension-declaration.hh"
#include "swift/syntax/function-declaration.hh"
#include "swift/syntax/initializer-declaration.hh"
#include "swift/syntax/protocol-declaration.hh"
#include "swift/syntax/source-file.hh"
#include "swift/syntax/struct-declaration.hh"
#include "swift/syntax/subscript-declaration.hh"
#include "swift/syntax/top-level-declaration.hh"

#include <cassert>

namespace swift {
namespace ast {
void *declaration::operator new(size_t size, const ast::context &context,
                                ast::declaration_context *declaration_context,
                                size_t extra) {
  assert(declaration_context == nullptr or
         &declaration_context->ast_context() == &context);
  return ::operator new(size + extra, context);
}

declaration *
declaration::from_declaration_context(const ast::declaration_context *context) {
  const ast::declaration *declaration = nullptr;
  switch (context->type()) {
  default:
    swift_unreachable("declaration type is an unknown declaration context");
  case declaration_context::type::class_declaration:
    declaration = static_cast<const ast::class_declaration *>(context);
    break;
  case declaration_context::type::enum_declaration:
    declaration = static_cast<const ast::enum_declaration *>(context);
    break;
  case declaration_context::type::extension_declaration:
    declaration = static_cast<const ast::extension_declaration *>(context);
    break;
  case declaration_context::type::function_declaration:
    declaration = static_cast<const ast::function_declaration *>(context);
    break;
  case declaration_context::type::initializer_declaration:
    declaration = static_cast<const ast::initializer_declaration *>(context);
    break;
  case declaration_context::type::protocol_declaration:
    declaration = static_cast<const ast::protocol_declaration *>(context);
    break;
  case declaration_context::type::struct_declaration:
    declaration = static_cast<const ast::struct_declaration *>(context);
    break;
  case declaration_context::type::subscript_declaration:
    declaration = static_cast<const ast::subscript_declaration *>(context);
    break;

  case declaration_context::type::source_file:
    return nullptr;
  case declaration_context::type::top_level_declaration:
    declaration = static_cast<const ast::top_level_declaration *>(context);
    break;
  }
  return const_cast<ast::declaration *>(declaration);
}

ast::context &declaration::ast_context() {
  return source_file()->ast_context();
}

ast::source_file *declaration::source_file() {
  ast::declaration_context *declaration_context = declaration_context_;
  assert(declaration_context &&
         "declaration is not contained within a context");

  while (not declaration_context->is_source_file()) {
    declaration_context = declaration_context->parent();
    assert(declaration_context &&
           "declaration is not contained within a context");
  }

  return static_cast<ast::source_file *>(declaration_context);
}
}
}

