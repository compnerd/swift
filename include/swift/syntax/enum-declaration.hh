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

#ifndef swift_syntax_enum_declaration_hh
#define swift_syntax_enum_declaration_hh

#include "swift/syntax/declaration.hh"
#include "swift/syntax/declaration-context.hh"

#include <vector>
#include <ext/string_view>

namespace swift::ast {
class enumeration_element_declaration;

class enum_declaration : public declaration, public declaration_context {
  std::u32string_view name_;
  std::vector<ast::declaration *> members_;

public:
  enum_declaration(ast::declaration_context *declaration_context,
                   std::u32string_view enum_name,
                   const std::vector<ast::declaration *> &members)
      : ast::declaration(declaration::type::enum_declaration,
                         declaration_context),
        ast::declaration_context(declaration_context::type::enum_declaration),
        name_(enum_name), members_(members) {}

  std::u32string_view name() const {
    return name_;
  }
  std::vector<const ast::declaration *> members() const noexcept {
    return std::vector<const ast::declaration *>(members_.begin(),
                                                 members_.end());
  }
};
}

#endif

