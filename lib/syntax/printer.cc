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

#include "swift/syntax/printer.hh"
#include "swift/support/ucs4-support.hh"

#include <llvm/ADT/SmallString.h>

namespace swift::ast {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"

void printer::print(const ast::statement *statement) {
  if (statement == nullptr)
    return;

  indent_ = indent_ + shift_width;
  os_ << std::endl;
  statement->accept(*this);
  indent_ = indent_ - shift_width;
}

void printer::print(const ast::pattern *pattern) {
  if (pattern == nullptr)
    return;

  indent_ = indent_ + shift_width;
  os_ << std::endl;
  pattern->accept(*this);
  indent_ = indent_ - shift_width;
}

void printer::print(const ast::type *type) {
  if (type == nullptr)
    return;

  indent_ = indent_ + shift_width;
  os_ << std::endl;
  type->accept(*this);
  indent_ = indent_ - shift_width;
}

void printer::print(const char *name,
                    const std::vector<const ast::pattern *> &patterns) {
  if (patterns.empty())
    return;

  indent_ = indent_ + shift_width;
  os_ << std::endl;
  printer::scope scope(*this, name);
  for (const auto pattern : patterns)
    print(pattern);
  indent_ = indent_ - shift_width;
}

void printer::print(const char *name,
                    const std::vector<const ast::statement *> &statements) {
  if (statements.empty())
    return;

  indent_ = indent_ + shift_width;
  os_ << std::endl;
  printer::scope scope(*this, name);
  for (const auto statement : statements)
    print(statement);
  indent_ = indent_ - shift_width;
}

void printer::print_quoted(std::u32string_view string, char quote) {
  os_ << quote << string << quote;
}

void printer::print_expression(const swift::ast::expression *expression) {
  if (expression)
    return print(expression);
  os_ << '\n';
  printer::scope(*this, "**NULL EXPRESSION**");
}

void printer::visit(const source_file &) {
  printer::scope scope(*this, "source_file");
}

void printer::visit(const top_level_declaration &) {
  printer::scope scope(*this, "top_level_decl");
}

void printer::visit(const prefix_unary_expression &expression) {
  printer::scope scope(*this, "prefix_unary_expr");
  os_ << " type='<null type>'";
  print(expression.prefix_operator());
  print(expression.subexpression());
}

void printer::visit(const in_out_expression &expression) {
  printer::scope scope(*this, "inout_expr");
  os_ << " type='<null>'";
  print(expression.subexpression());
}

void printer::visit(const sequence_expression &expression) {
  printer::scope scope(*this, "sequence_expr");
  os_ << " type='<null type>'";
  for (const auto &subexpression : expression)
    print(subexpression);
}

void printer::visit(const postfix_unary_expression &expression) {
  printer::scope scope(*this, "postfix_unary_expr");
  os_ << " type='<null>'";
  print(expression.postfix_operator());
  print(expression.subexpression());
}

void printer::visit(const function_call_expression &expression) {
  printer::scope scope(*this, "call_expr");
  os_ << " type='<null>'";
  print(expression.function());
  print(expression.arguments());
}

void printer::visit(const initializer_expression &) {
  __builtin_trap();
}

void printer::visit(const explicit_member_expression &member) {
  printer::scope scope(*this,
                       member.resolved() ? "dot_expr" : "unresolved_dot_expr");
  os_ << " type='<null type>'";
  os_ << " field="; print_quoted(member.field(), '"');
  print(member.expression());
}

void printer::visit(const postfix_self_expression &postfix_self_expr) {
  printer::scope scope(*this, "dot_self_expr");
  os_ << " type='<null>'";
  print(postfix_self_expr.instance());
}

void printer::visit(const dynamic_type_expression &dynamic_type) {
  printer::scope scope(*this, "metatype_expr");
  os_ << " type='<null type>'";
  print(dynamic_type.expression());
}

void printer::visit(const subscript_expression &) {
  __builtin_trap();
}

void printer::visit(const forced_value_expression &forced_value_expr) {
  printer::scope scope(*this, "force_value_expression");
  os_ << " type='<null>'";
  print(forced_value_expr.expression());
}

void printer::visit(const optional_chaining_expression &) {
  __builtin_trap();
}

void printer::visit(const declaration_reference_expression &expression) {
  printer::scope scope(*this, expression.resolved()
                                  ? "decl_ref_expr"
                                  : "unresolved_decl_ref_expr");
  os_ << " type='<null>'"
      << " name=" << expression.name()
      << " specialized=no";
}

void printer::visit(const superclass_expression &) {
  printer::scope scope(*this, "super_ref_expr");
  os_ << " type='<null>'";
}

void printer::visit(const closure_expression &closure) {
  printer::scope(*this, "closure_expr");
  os_ << " type='<null>>'"
      << " descriminator=0";
  print(closure.body());
}

void printer::visit(const parenthesized_expression &expression) {
  const auto &elements = expression.elements();
  printer::scope scope(*this,
                       elements.size() == 1 ? "paren_expr" : "tuple_expr");
  os_ << " type='<null>'";
  for (const auto element : expression.elements())
    print(element);
}

void printer::visit(const implicit_member_expression &implicit_member_expr) {
  printer::scope scope(*this, "unresolved_member_expr");
  os_ << " type='<null>'"
      << " name="; print_quoted(implicit_member_expr.name(), '"');
}

void printer::visit(const wildcard_expression &) {
  printer::scope scope(*this, "discard_assignment_expression");
  os_ << " type='<null>'";
}

void printer::visit(const assignment_expression &assignment_expr) {
  printer::scope scope(*this, "assign_expr");
  print_expression(assignment_expr.lhs());
  print_expression(assignment_expr.rhs());
}

void printer::visit(const conditional_expression &conditional_expr) {
  printer::scope(*this, "if_expr");
  os_ << " type='<null>'";
  print_expression(conditional_expr.condition());
  print(conditional_expr.true_clause());
  print_expression(conditional_expr.false_clause());
}

void printer::visit(const is_subtype_expression &) {
  __builtin_trap();
}

void printer::visit(const checked_cast_expression &) {
  __builtin_trap();
}

void printer::visit(const conditional_checked_cast_expression &) {
  __builtin_trap();
}

void printer::visit(const boolean_literal_expression &literal) {
  printer::scope scope(*this, "boolean_literal_expr");
  os_ << " type='<null>'";
  os_ << " value=" << std::boolalpha << literal.value();
}

void printer::visit(const floating_point_literal_expression &literal) {
  llvm::SmallString<32> value;
  literal.value().toString(value);

  printer::scope scope(*this, "float_literal_expr");
  os_ << " type='<null>'";
  os_ << " value=" << value.str().str();
}

void printer::visit(const integer_literal_expression &literal) {
  printer::scope scope(*this, "integer_literal_expr");
  os_ << " type='<null>'";
  os_ << " value=" << literal.value().toString(10);
}

void printer::visit(const nil_literal_expression &) {
  printer::scope scope(*this, "nil_literal_expr");
  os_ << " type='<null>'";
}

void printer::visit(const string_literal_expression &literal) {
  printer::scope scope(*this, "string_literal_expr");
  os_ << " type='<null>'";
  os_ << " encoding=utf8";
  os_ << " value="; print_quoted(literal.value(), '"');
}

void printer::visit(const array_literal_expression &literal) {
  printer::scope scope(*this, "array_expr");
  os_ << " type='<null>'";
  print(literal.items());
}

void printer::visit(const dictionary_literal_expression &literal) {
  printer::scope scope(*this, "dictionary_expr");
  os_ << " type='<null>'";
  print(literal.items());
}

void printer::visit(const magic_literal_expression &literal) {
  printer::scope scope(*this, "magic_identifier_literal_expr");
  os_ << " type='<null>'";
  switch (literal.type()) {
  case magic_literal_expression::type::column:
    os_ << " kind=__COLUMN__";
    break;
  case magic_literal_expression::type::file:
    os_ << " kind=__FILE__";
    os_ << " encoding=utf8";
    break;
  case magic_literal_expression::type::function:
    os_ << " kind=__FUNCTION__";
    os_ << " encoding=utf8";
    break;
  case magic_literal_expression::type::line:
    os_ << " kind=__LINE__";
    break;
  }
}

void printer::visit(const import_declaration &declaration) {
  printer::scope scope(*this, "import_decl");
  os_ << " " << declaration.import_path();
}

void printer::visit(const constant_declaration &constant) {
  printer::scope scope(*this, "pattern_binding_decl");
  print(constant.name());
  print(constant.initializer());
}

void printer::visit(const variable_declaration &variable) {
  printer::scope scope(*this, "pattern_binding_decl");
  print(variable.name());
  print(variable.initializer());
}

void printer::visit(const typealias_declaration &typealias) {
  printer::scope scope(*this, "typealias");
  os_ << " "; print_quoted(typealias.alias(), '"');
  os_ << " type='" << typealias.type_name() << ".Type'";
  os_ << " access=internal";
  os_ << " type="; print_quoted(typealias.type_name(), '\'');
}

void printer::visit(const function_declaration &function) {
  printer::scope scope(*this, "func_decl");
  // TODO(compnerd) print SEL-type name
  os_ << " "; print_quoted(function.name(), '"');
  os_ << " type='<null type>'";
  print("body_params", function.parameter_clauses());
  print(function.result_type());
  for (const auto *decl : static_cast<const declaration_context &>(function))
    print(decl);
}

void printer::visit(const enum_declaration &) {
  __builtin_trap();
}

void printer::visit(const struct_declaration &struct_decl) {
  printer::scope scope(*this, "struct_decl");
  os_ << " "; print_quoted(struct_decl.name(), '"');
  os_ << " type='<null type>'";
  for (const auto *decl : static_cast<const declaration_context &>(struct_decl))
    print(decl);
}

void printer::visit(const class_declaration &class_decl) {
  printer::scope scope(*this, "class_decl");
  os_ << " "; print_quoted(class_decl.name(), '"');
  os_ << " type='<null type>'";
  for (const auto *decl : static_cast<const declaration_context &>(class_decl))
    print(decl);
}

void printer::visit(const protocol_declaration &protocol) {
  printer::scope scope(*this, "protocol");
  os_ << " "; print_quoted(protocol.name(), '"');
  os_ << " type='<null type>'";
}

void printer::visit(const initializer_declaration &initializer_decl) {
  printer::scope scope(*this, "unresolved_constructor");
  os_ << " type='<null>'";
  print(initializer_decl.body());
}

void printer::visit(const deinitializer_declaration &) {
  __builtin_trap();
}

void printer::visit(const extension_declaration &) {
  __builtin_trap();
}

void printer::visit(const subscript_declaration &) {
  __builtin_trap();
}

void printer::visit(const operator_declaration &) {
  __builtin_trap();
}

void printer::visit(const for_statement &) {
  __builtin_trap();
}

void printer::visit(const for_in_statement &for_each_stmt) {
  printer::scope(*this, "for_each_stmt");
  print(for_each_stmt.item());
  print(for_each_stmt.collection());
  print(for_each_stmt.body());
}

void printer::visit(const while_statement &while_stmt) {
  printer::scope scope(*this, "while_stmt");
  print(while_stmt.condition());
  print(while_stmt.body());
}

void printer::visit(const repeat_while_statement &repeat_while_stmt) {
  printer::scope scope(*this, "do_while_stmt");
  print(repeat_while_stmt.body());
  print(repeat_while_stmt.condition());
}

void printer::visit(const if_statement &if_stmt) {
  printer::scope scope(*this, "if_stmt");
  print(if_stmt.condition());
  print(if_stmt.true_clause());
  if (auto false_clause = if_stmt.false_clause())
    print(false_clause);
}

void printer::visit(const guard_statement &) {
  __builtin_trap();
}

void printer::visit(const switch_statement &switch_stmt) {
  printer::scope switch_scope(*this, "switch_stmt");
  print(switch_stmt.control_expression());

  for (const auto switch_case : switch_stmt.cases()) {
    std::vector<std::tuple<ast::pattern *, ast::expression *>> case_item_list;
    ast::statement *body;

    std::tie(case_item_list, body) = switch_case;

    indent_ = indent_ + shift_width;
    os_ << std::endl;
    printer::scope case_scope(*this, "case_stmt");
    for (const auto case_item : case_item_list) {
        ast::pattern *pattern;
        ast::expression *guard;

        std::tie(pattern, guard) = case_item;

        indent_ = indent_ + shift_width;
        os_ << std::endl;
        printer::scope case_label_scope(*this, "case_label_item");
        indent_ = indent_ - shift_width;
        print(pattern);
    }
    print(body);
  }
}

void printer::visit(const labelled_statement &labelled_stmt) {
  print(labelled_stmt.statement());
}

void printer::visit(const break_statement &) {
  printer::scope scope(*this, "break_stmt");
}

void printer::visit(const continue_statement &) {
  printer::scope scope(*this, "continue_stmt");
}

void printer::visit(const fallthrough_statement &) {
  __builtin_trap();
}

void printer::visit(const return_statement &return_stmt) {
  printer::scope scope(*this, "return_stmt");
  if (return_stmt.value())
    print(return_stmt.value());
}

void printer::visit(const throw_statement &) {
  __builtin_trap();
}

void printer::visit(const do_statement &do_stmt) {
  printer::scope scope(*this, do_stmt.catch_clauses().size() ? "do_catch_stmt"
                                                             : "do_stmt");
  print(do_stmt.body());
  for (const auto catch_clause : do_stmt.catch_clauses()) {
    printer::scope scope(*this, "catch");
    print(std::get<0>(catch_clause));
    print(std::get<1>(catch_clause));
  }
}

void printer::visit(const defer_statement &) {
  __builtin_trap();
}

void printer::visit(const build_configuration_statement &) {
  __builtin_trap();
}

void printer::visit(const line_control_statement &) {
}

void printer::visit(const ast::statements &statements) {
  printer::scope scope(*this, "brace_stmt");
  for (const auto substatement : statements.substatements())
    print(substatement);
}

void printer::visit(const pattern_any &) {
  printer::scope scope(*this, "pattern_any");
}

void printer::visit(const pattern_expression &pattern) {
  printer::scope scope(*this, "pattern_expr");
  print(pattern.expression());
}

void printer::visit(const pattern_named &pattern) {
  printer::scope scope(*this, "pattern_named");
  os_ << (pattern.implicit() ? " implicit " : " ");
  print_quoted(pattern.name().empty() ? U"_" : pattern.name(), '\'');
}

void printer::visit(const pattern_tuple &pattern) {
  printer::scope scope(*this, pattern.elements().size() == 1 ? "pattern_paren"
                                                             : "pattern_tuple");
  for (const auto element : pattern.elements())
    print(element);
}

void printer::visit(const pattern_typed &pattern) {
  printer::scope scope(*this, "pattern_typed");
  print(pattern.pattern());
  print(pattern.pattern_type());
}

void printer::visit(const pattern_var &pattern) {
  printer::scope scope(*this, "pattern_let");
  print(pattern.pattern());
}

void printer::visit(const type_array &array) {
  printer::scope scope(*this, "type_array");
  print(array.type());
}

void printer::visit(const type_composite &) {
  __builtin_trap();
}

void printer::visit(const type_dictionary &dictionary) {
  printer::scope scope(*this, "type_dictionary");
  print(dictionary.key_type());
  print(dictionary.value_type());
}

void printer::visit(const type_function &function) {
  printer::scope scope(*this, "type_function");
  print(function.parameter_type());
  print(function.return_type());
}

void printer::visit(const type_identifier &identifer) {
  printer::scope scope(*this, "type_ident");
  indent_ = indent_ + shift_width;
  for (const auto component : identifer.components()) {
    os_ << std::endl;
    printer::scope scope(*this, "component");
    os_ << " id="; print_quoted(component, '\'');
    os_ << " bind=none";
  }
  indent_ = indent_ - shift_width;
}

void printer::visit(const type_inout &inout) {
  printer::scope scope(*this, "type_inout");
  print(inout.type());
}

void printer::visit(const type_metatype &metatype) {
  printer::scope scope(*this, "type_metatype");
  print(metatype.type());
}

void printer::visit(const type_tuple &tuple) {
  printer::scope scope(*this, "type_tuple");
  for (const auto element : tuple.elements())
    print(element);
}
#pragma clang diagnostic pop
}

