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

#ifndef swift_syntax_ast_printer_hh
#define swift_syntax_ast_printer_hh

#include "swift/syntax/visitor.hh"

#include <ostream>
#include <string>

namespace swift {
namespace ast {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Wignored-qualifiers"

class printer : public ast::visitor<printer> {
  static const unsigned shift_width = 2;

  std::ostream &os_;
  unsigned indent_;

  class scope {
    printer &printer_;
  public:
    scope(printer &printer, const char *name) : printer_(printer) {
      printer_.os_ << std::string(printer.indent_, ' ') << '(' << name;
    }
    ~scope() {
      printer_.os_ << ')';
    }
  };

  void print(const ast::statement *statement);
  void print(const ast::pattern *pattern);
  void print(const ast::type *type);
  void print(const char *name,
             const std::vector<const ast::pattern *> &patterns);
  void print(const char *name,
             const std::vector<const ast::statement *> &statements);

  void print_quoted(std::u32string_view string, char quote);
  void print_quoted(const std::string &string, char quote);

  void print_expression(const ast::expression *expression);

public:
  printer(std::ostream &os, unsigned indent = 0) : os_(os), indent_(indent) {}

  void visit(const source_file &) override;
  void visit(const top_level_declaration &) override;

  void visit(const prefix_unary_expression &) override;
  void visit(const in_out_expression &) override;
  void visit(const sequence_expression &) override;
  void visit(const postfix_unary_expression &) override;
  void visit(const function_call_expression &) override;
  void visit(const initializer_expression &) override;
  void visit(const explicit_member_expression &) override;
  void visit(const postfix_self_expression &) override;
  void visit(const dynamic_type_expression &) override;
  void visit(const subscript_expression &) override;
  void visit(const forced_value_expression &) override;
  void visit(const optional_chaining_expression &) override;
  void visit(const declaration_reference_expression &) override;
  void visit(const superclass_expression &) override;
  void visit(const closure_expression &) override;
  void visit(const parenthesized_expression &) override;
  void visit(const implicit_member_expression &) override;
  void visit(const wildcard_expression &) override;
  void visit(const assignment_expression &) override;
  void visit(const conditional_expression &) override;

  void visit(const is_subtype_expression &) override;
  void visit(const checked_cast_expression &) override;
  void visit(const conditional_checked_cast_expression &) override;

  void visit(const boolean_literal_expression &) override;
  void visit(const floating_point_literal_expression &) override;
  void visit(const integer_literal_expression &) override;
  void visit(const nil_literal_expression &) override;
  void visit(const string_literal_expression &) override;
  void visit(const array_literal_expression &) override;
  void visit(const dictionary_literal_expression &) override;
  void visit(const magic_literal_expression &) override;

  void visit(const import_declaration &) override;
  void visit(const constant_declaration &) override;
  void visit(const variable_declaration &) override;
  void visit(const typealias_declaration &) override;
  void visit(const function_declaration &) override;
  void visit(const enum_declaration &) override;
  void visit(const struct_declaration &) override;
  void visit(const class_declaration &) override;
  void visit(const protocol_declaration &) override;
  void visit(const initializer_declaration &) override;
  void visit(const deinitializer_declaration &) override;
  void visit(const extension_declaration &) override;
  void visit(const subscript_declaration &) override;
  void visit(const operator_declaration &) override;

  void visit(const for_statement &) override;
  void visit(const for_in_statement &) override;
  void visit(const while_statement &) override;
  void visit(const repeat_while_statement &) override;

  void visit(const if_statement &) override;
  void visit(const guard_statement &) override;
  void visit(const switch_statement &) override;

  void visit(const labelled_statement &) override;

  void visit(const break_statement &) override;
  void visit(const continue_statement &) override;
  void visit(const fallthrough_statement &) override;
  void visit(const return_statement &) override;
  void visit(const throw_statement &) override;

  void visit(const do_statement &) override;
  void visit(const defer_statement &) override;

  void visit(const build_configuration_statement &) override;
  void visit(const line_control_statement &) override;

  void visit(const statements &) override;

  void visit(const pattern_any &) override;
  void visit(const pattern_expression &) override;
  void visit(const pattern_named &) override;
  void visit(const pattern_tuple &) override;
  void visit(const pattern_typed &) override;
  void visit(const pattern_var &) override;

  void visit(const type_array &) override;
  void visit(const type_composite &) override;
  void visit(const type_dictionary &) override;
  void visit(const type_function &) override;
  void visit(const type_identifier &) override;
  void visit(const type_inout &) override;
  void visit(const type_metatype &) override;
  void visit(const type_tuple &) override;
};

#pragma clang diagnostic pop

}
}

#endif

