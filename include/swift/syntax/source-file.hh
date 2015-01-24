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

#ifndef swift_syntax_source_file_hh
#define swift_syntax_source_file_hh

#include "swift/syntax/declaration-context.hh"

#include <string>

namespace swift {
namespace ast {
class context;

class source_file : public declaration_context {
  ast::context &ast_context_;
  std::string name_;

  void *operator new(size_t size, const ast::context &context,
                     ast::declaration_context *declaration_context);

  explicit source_file(ast::context &context, const std::string &name)
      : ast::declaration_context(declaration_context::type::source_file),
        ast_context_(context), name_(name) {}

public:
  virtual ~source_file();

  static source_file *create(ast::context &context, const std::string &name);

  ast::context &ast_context() const {
    return ast_context_;
  }
  const std::string &name() const {
    return name_;
  }

  ast::declaration_context *declaration_context() {
    return static_cast<ast::declaration_context *>(this);
  }

  void dump() const;
};
}
}

#endif

