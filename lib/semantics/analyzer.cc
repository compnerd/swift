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

#include "swift/semantic/analyzer.hh"

#include "swift/diagnostics/diagnostics.hh"

#include "swift/support/ucs4-support.hh"

#include "swift/syntax/context.hh"

#include "swift/syntax/source-file.hh"

#include "swift/syntax/statement.hh"

#include "swift/syntax/expression.hh"
#include "swift/syntax/declaration.hh"
#include "swift/syntax/loop-statement.hh"
#include "swift/syntax/branch-statement.hh"
#include "swift/syntax/control-transfer-statement.hh"

#include "swift/syntax/loop-statement.hh"
#include "swift/syntax/branch-statement.hh"
#include "swift/syntax/labelled-statement.hh"
#include "swift/syntax/control-transfer-statement.hh"

#include "swift/syntax/prefix-unary-expression.hh"
#include "swift/syntax/in-out-expression.hh"
#include "swift/syntax/sequence-expression.hh"
#include "swift/syntax/assignment-expression.hh"
#include "swift/syntax/conditional-expression.hh"
#include "swift/syntax/type-casting-expression.hh"
#include "swift/syntax/literal-expression.hh"
#include "swift/syntax/superclass-expression.hh"
#include "swift/syntax/closure-expression.hh"
#include "swift/syntax/parenthesized-expression.hh"
#include "swift/syntax/implicit-member-expression.hh"
#include "swift/syntax/wildcard-expression.hh"
#include "swift/syntax/postfix-unary-expression.hh"
#include "swift/syntax/function-call-expression.hh"
#include "swift/syntax/initializer-expression.hh"
#include "swift/syntax/explicit-member-expression.hh"
#include "swift/syntax/postfix-self-expression.hh"
#include "swift/syntax/dynamic-type-expression.hh"
#include "swift/syntax/subscript-expression.hh"
#include "swift/syntax/forced-value-expression.hh"
#include "swift/syntax/optional-chaining-expression.hh"
#include "swift/syntax/declaration-reference-expression.hh"

#include "swift/syntax/is-subtype-expression.hh"
#include "swift/syntax/checked-cast-expression.hh"
#include "swift/syntax/conditional-checked-cast-expression.hh"

#include "swift/syntax/boolean-literal-expression.hh"
#include "swift/syntax/floating-point-literal-expression.hh"
#include "swift/syntax/integer-literal-expression.hh"
#include "swift/syntax/nil-literal-expression.hh"
#include "swift/syntax/string-literal-expression.hh"
#include "swift/syntax/array-literal-expression.hh"
#include "swift/syntax/dictionary-literal-expression.hh"
#include "swift/syntax/magic-literal-expression.hh"

#include "swift/syntax/import-declaration.hh"
#include "swift/syntax/constant-declaration.hh"
#include "swift/syntax/variable-declaration.hh"
#include "swift/syntax/typealias-declaration.hh"
#include "swift/syntax/function-declaration.hh"
#include "swift/syntax/enum-declaration.hh"
#include "swift/syntax/struct-declaration.hh"
#include "swift/syntax/class-declaration.hh"
#include "swift/syntax/protocol-declaration.hh"
#include "swift/syntax/initializer-declaration.hh"
#include "swift/syntax/deinitializer-declaration.hh"
#include "swift/syntax/extension-declaration.hh"
#include "swift/syntax/subscript-declaration.hh"
#include "swift/syntax/operator-declaration.hh"

#include "swift/syntax/for-statement.hh"
#include "swift/syntax/for-in-statement.hh"
#include "swift/syntax/while-statement.hh"
#include "swift/syntax/repeat-while-statement.hh"

#include "swift/syntax/if-statement.hh"
#include "swift/syntax/switch-statement.hh"

#include "swift/syntax/break-statement.hh"
#include "swift/syntax/continue-statement.hh"
#include "swift/syntax/fallthrough-statement.hh"
#include "swift/syntax/return-statement.hh"
#include "swift/syntax/defer-statement.hh"

#include "swift/syntax/build-configuration-statement.hh"
#include "swift/syntax/line-control-statement.hh"

#include "swift/syntax/statements.hh"

#include "swift/syntax/pattern.hh"
#include "swift/syntax/pattern-any.hh"
#include "swift/syntax/pattern-expression.hh"
#include "swift/syntax/pattern-named.hh"
#include "swift/syntax/pattern-tuple.hh"
#include "swift/syntax/pattern-typed.hh"
#include "swift/syntax/pattern-var.hh"

#include "swift/syntax/type-array.hh"
#include "swift/syntax/type-composite.hh"
#include "swift/syntax/type-dictionary.hh"
#include "swift/syntax/type-function.hh"
#include "swift/syntax/type-identifier.hh"
#include "swift/syntax/type-inout.hh"
#include "swift/syntax/type-metatype.hh"
#include "swift/syntax/type-tuple.hh"

#include <llvm/ADT/StringRef.h>

#include <sstream>

using namespace swift::ast;

using diagnostic = swift::diagnostics::diagnostic;

static int hex_digit_value(char32_t ch) {
  if (ch >= U'0' and ch <= U'9') return ch - U'0';
  if (ch >= U'a' and ch <= U'f') return ch - U'a';
  if (ch >= U'A' and ch <= U'F') return ch - U'A';
  return -1;
}

static bool get_integer_value(std::u32string_view literal, unsigned radix,
                              llvm::APInt &integer) {
  uint64_t value = 0;
  for (const char32_t digit : literal)
    value = value * radix + hex_digit_value(digit);
  integer = value;
  return not (integer.getZExtValue() == value);
}

namespace swift::semantic {
analyzer::analyzer(ast::context &ast_context)
    : scope_(nullptr, scope::type::top_level), ast_context_(ast_context),
      diagnostics_engine_(ast_context.diagnostics_engine()) {
  // FIXME(compnerd) this really should be done lazily
  declaration_context_ = ast_context_.source_file()->declaration_context();
}

ast::expression *
analyzer::prefix_unary_expression(ast::expression *prefix_operator,
                                  ast::expression *subexpression) {
  return new (ast_context_)
      ast::prefix_unary_expression(prefix_operator, subexpression);
}

expression *analyzer::in_out_expression(ast::expression *expression) {
  return new (ast_context_) ast::in_out_expression(expression);
}

ast::expression *
analyzer::sequence_expression(const std::vector<ast::expression *> &exprs) {
  if (scope_.is<scope::type::top_level>()) {
    diagnose(location(),
             diagnostic::err_expressions_not_allowed_at_the_top_level);
    return nullptr;
  }

  return new (ast_context_) ast::sequence_expression(exprs);
}

ast::expression *
analyzer::postfix_unary_expression(ast::expression *subexpression,
                                   ast::expression *postfix_operator) {
  return new (ast_context_)
      ast::postfix_unary_expression(subexpression, postfix_operator);
}

ast::expression *
analyzer::assignment_expression(ast::expression *lhs, ast::expression *rhs) {
  return new (ast_context_) ast::assignment_expression(lhs, rhs);
}

ast::expression *analyzer::conditional_expression(ast::expression *condition,
                                                  ast::expression *if_true,
                                                  ast::expression *if_false) {
  return new (ast_context_)
      ast::conditional_expression(condition, if_true, if_false);
}

ast::expression *analyzer::is_subtype_expression() {
  return new (ast_context_) ast::is_subtype_expression();
}

ast::expression *analyzer::checked_cast_expression() {
  return new (ast_context_) ast::checked_cast_expression();
}

ast::expression *analyzer::conditional_checked_cast_expression() {
  return new (ast_context_) ast::conditional_checked_cast_expression();
}

expression *
analyzer::function_call(ast::expression *function, ast::expression *arguments) {
  return new (ast_context_) function_call_expression(function, arguments);
}

ast::expression *
analyzer::initializer_expression(ast::expression *declaration) {
  return new (ast_context_) ast::initializer_expression(declaration);
}

ast::expression *
analyzer::explicit_member_expression(ast::expression *expression,
                                     std::u32string_view member) {
  return new (ast_context_) ast::explicit_member_expression(expression, member);
}

ast::expression *analyzer::postfix_self_expression(ast::expression *instance) {
  return new (ast_context_) ast::postfix_self_expression(instance);
}

ast::expression *
analyzer::dynamic_type_expression(ast::expression *expression) {
  return new (ast_context_) ast::dynamic_type_expression(expression);
}

ast::expression *analyzer::subscript_expression() {
  return new (ast_context_) ast::subscript_expression();
}

ast::expression *
analyzer::forced_value_expression(ast::expression *expression) {
  return new (ast_context_) ast::forced_value_expression(expression);
}

ast::expression *analyzer::optional_chaining_expression() {
  return new (ast_context_) ast::optional_chaining_expression();
}

ast::expression *
analyzer::declaration_reference_expression(std::u32string_view value) {
  return new (ast_context_) ast::declaration_reference_expression(value);
}

expression *analyzer::boolean_literal_expression(const swift::token &token) {
  assert(token.is<token::type::literal>() and
         (token.is<token::literal_type::constant_true>() or
          token.is<token::literal_type::constant_false>()) &&
         "unknown swift boolean value");

  bool value = token.is<token::literal_type::constant_true>();
  return new (ast_context_) ast::boolean_literal_expression(value);
}

expression *
analyzer::floating_point_literal_expression(std::u32string_view literal) {
  std::ostringstream oss;
  oss << literal;

  llvm::APFloat value(llvm::APFloat::IEEEquad,
                      llvm::StringRef(oss.str().c_str()));
  return new (ast_context_) ast::floating_point_literal_expression(value);
}

expression *analyzer::integer_literal_expression(std::u32string_view literal) {
  llvm::APSInt value(64);
  get_integer_value(literal, 10, value);
  return new (ast_context_) ast::integer_literal_expression(value);
}

expression *analyzer::nil_literal_expression() {
  return new (ast_context_) ast::nil_literal_expression();
}

expression *analyzer::string_literal_expression(std::u32string_view value) {
  return new (ast_context_) ast::string_literal_expression(value);
}

ast::expression *analyzer::array_literal_expression(ast::expression *items) {
  return new (ast_context_) ast::array_literal_expression(items);
}

ast::expression *
analyzer::dictionary_literal_expression(ast::expression *items) {
  return new (ast_context_) ast::dictionary_literal_expression(items);
}

expression *analyzer::magic_literal_expression(const token &literal) {
  switch (literal) {
  default:
    swift_unreachable("invalid magic literal type");
  case token::type::kw___FILE__:
    return new (ast_context_)
        ast::magic_literal_expression(magic_literal_expression::type::file,
                                      nullptr);
  case token::type::kw___FUNCTION__:
    return new (ast_context_)
        ast::magic_literal_expression(magic_literal_expression::type::function,
                                      nullptr);
  case token::type::kw___LINE__:
    return new (ast_context_)
        ast::magic_literal_expression(magic_literal_expression::type::line,
                                      literal.location().start().line());
  case token::type::kw___COLUMN__:
    return new (ast_context_)
        ast::magic_literal_expression(magic_literal_expression::type::column,
                                      literal.location().start().column());
  }
}

ast::expression *analyzer::superclass_expression() {
  return new (ast_context_) ast::superclass_expression();
}

ast::expression *
analyzer::closure_expression(ast::statement *statements) {
  return new (ast_context_) ast::closure_expression(statements);
}

expression *analyzer::parenthesized_expression(
    const std::vector<ast::expression *> &elements) {
  return new (ast_context_) ast::parenthesized_expression(elements);
}

ast::expression *
analyzer::implicit_member_expression(std::u32string_view member) {
  return new (ast_context_) ast::implicit_member_expression(member);
}

ast::expression *analyzer::wildcard_expression() {
  return new (ast_context_) ast::wildcard_expression();
}

ast::declaration *
analyzer::import_declaration(range declaration_range,
                             std::u32string_view import_path) {
  if (not scope_.is<scope::type::top_level>()) {
    diagnose(declaration_range,
             diagnostic::err_declaration_is_only_valid_at_file_scope);
    return nullptr;
  }

  return new (ast_context_, declaration_context_)
      ast::import_declaration(declaration_context_, import_path);
}

ast::declaration *analyzer::constant_declaration(ast::pattern *name,
                                                 ast::expression *initializer) {
  return new (ast_context_, declaration_context_)
      ast::constant_declaration(declaration_context_, name, initializer);
}

ast::declaration *analyzer::variable_declaration(ast::pattern *name,
                                                 ast::expression *initializer) {
  return new (ast_context_, declaration_context_)
      ast::variable_declaration(declaration_context_, name, initializer);
}

ast::declaration *analyzer::typealias_declaration(std::u32string_view alias,
                                                  std::u32string_view type) {
  return new (ast_context_, declaration_context_)
      ast::typealias_declaration(declaration_context_, alias, type);
}

ast::declaration *
analyzer::function_declaration(std::u32string_view function_name,
                               const std::vector<ast::pattern *> &parameters,
                               ast::type *result_type,
                               ast::statement *body) {
  return new (ast_context_, declaration_context_)
      ast::function_declaration(declaration_context_, function_name, parameters,
                                result_type, body);
}

ast::declaration *
analyzer::enum_declaration(std::u32string_view enumeration_name,
                           const std::vector<ast::declaration *> &elements) {
  return new (ast_context_, declaration_context_)
      ast::enum_declaration(declaration_context_, enumeration_name, elements);
}

ast::declaration *
analyzer::struct_declaration(std::u32string_view struct_name,
                             ast::statement *declarations) {
  return new (ast_context_, declaration_context_)
      ast::struct_declaration(declaration_context_, struct_name, declarations);
}

ast::declaration *
analyzer::class_declaration(std::u32string_view class_name,
                            ast::statement *statements) {
  return new (ast_context_, declaration_context_)
      ast::class_declaration(declaration_context_, class_name, statements);
}

ast::declaration *
analyzer::protocol_declaration(std::u32string_view protocol_name,
                               ast::statement *members) {
  return new (ast_context_, declaration_context_)
      ast::protocol_declaration(declaration_context_, protocol_name, members);
}

ast::declaration *
analyzer::initializer_declaration(ast::pattern *parameters,
                                  ast::statement *body) {
  return new (ast_context_, declaration_context_)
      ast::initializer_declaration(declaration_context_, parameters, body);
}

ast::declaration *
analyzer::deinitializer_declaration(ast::statement *body) {
  return new (ast_context_, declaration_context_)
      ast::deinitializer_declaration(declaration_context_, body);
}

ast::declaration *
analyzer::extension_declaration(std::u32string_view type_name,
                                const std::vector<std::u32string_view> &adopted_protocols,
                                ast::statement *body) {
  return new (ast_context_, declaration_context_)
      ast::extension_declaration(declaration_context_, type_name,
                                 adopted_protocols, body);
}

ast::declaration *analyzer::subscript_declaration(ast::pattern *parameters,
                                                  ast::type *return_type,
                                                  ast::declaration *getter,
                                                  ast::declaration *setter) {
  return new (ast_context_, declaration_context_)
      ast::subscript_declaration(declaration_context_, parameters, return_type,
                                 getter, setter);
}

ast::declaration *
analyzer::operator_declaration(token type, std::u32string_view operator_name,
                               uint8_t precedence, token associativity) {
  enum operator_declaration::type operator_type;
  enum operator_declaration::associativity operator_associativity;

  switch (type) {
  default:
    swift_unreachable("invalid operator type token");
  case token::type::kw_infix:
    operator_type = operator_declaration::type::infix;
    break;
  case token::type::kw_prefix:
    operator_type = operator_declaration::type::prefix;
    break;
  case token::type::kw_postfix:
    operator_type = operator_declaration::type::postfix;
    break;
  }

  switch (associativity) {
  default:
    swift_unreachable("invalid operator associativity token");
  case token::type::kw_none:
    operator_associativity = operator_declaration::associativity::none;
    break;
  case token::type::kw_left:
    operator_associativity = operator_declaration::associativity::left;
    break;
  case token::type::kw_right:
    operator_associativity = operator_declaration::associativity::right;
    break;
  }

  if (operator_type == operator_declaration::type::prefix and
      operator_name == U"&") {
    // TODO(compnerd) use the location of the operator_name
    diagnose(location(),
             diagnostic::err_cannot_declare_a_custom_prefix_name_operator)
        << operator_name;
    return nullptr;
  }
  if (operator_type == operator_declaration::type::postfix and
      operator_name == U"!") {
    // TODO(compnerd) use the location of the operator_name
    diagnose(location(),
             diagnostic::err_cannot_declare_a_custom_prefix_name_operator)
        << operator_name;
    return nullptr;
  }

  return new (ast_context_, declaration_context_)
      ast::operator_declaration(declaration_context_, operator_type,
                                operator_name, precedence,
                                operator_associativity);
}

/* statement constructors */
ast::statement *
analyzer::statements(std::vector<statement *> &statements) {
  return new (ast_context_) ast::statements(statements);
}

/* loop statement constructors */
ast::statement *analyzer::for_statement(ast::statement *initializer,
                                        ast::expression *condition,
                                        ast::expression *increment,
                                        ast::statement *body) {
  assert((initializer->type() == statement::type::declaration or
          initializer->type() == statement::type::expression) &&
         "initializer must be declaration or expression");

  return new (ast_context_)
      ast::for_statement(initializer, condition, increment, body);
}

ast::statement *analyzer::for_in_statement(ast::pattern *item,
                                           ast::expression *collection,
                                           ast::statement *statements) {
  // TODO(compnerd) ensure that collection conforms to the SequenceType protocol
  // TODO(compnerd) ensure that collection conforms to the GeneratorType
  // protocol
  return new (ast_context_) ast::for_in_statement(item, collection, statements);
}

ast::statement *analyzer::while_statement(ast::statement *condition,
                                          ast::statement *body) {
  // TODO(compnerd) ensure that condition conforms to the BooleanType protocol
  return new (ast_context_) ast::while_statement(condition, body);
}

/* branch statement constructors */
ast::statement *analyzer::if_statement(ast::statement *condition,
                                       ast::statement *true_clause,
                                       ast::statement *false_clause) {
  // TODO(compnerd) ensure that condition conforms to BooleanType protocol
  return new (ast_context_)
      ast::if_statement(condition, true_clause, false_clause);
}

ast::statement *
analyzer::switch_statement(ast::expression * control_expression,
                           std::vector<ast::switch_statement::case_item> &case_statements) {
  return new (ast_context_)
      ast::switch_statement(control_expression, case_statements);
}

/* labelled statement constructor */
ast::statement *
analyzer::labelled_statement(std::u32string_view label,
                             ast::statement *loop_or_switch_statement) {
  switch (loop_or_switch_statement->type()) {
  default:
    swift_unreachable("invalid labelled statement");
  case ast::statement::type::loop_statement: {
    auto loop_statement =
        static_cast<ast::loop_statement *>(loop_or_switch_statement);

    return new (ast_context_) ast::labelled_statement(label, loop_statement);
  }
  case ast::statement::type::branch_statement: {
    auto branch_statement =
        static_cast<ast::branch_statement *>(loop_or_switch_statement);
    switch (branch_statement->type()) {
    default:
      break;
    case ast::branch_statement::type::switch_statement: {
      auto switch_statement =
          static_cast<ast::switch_statement *>(branch_statement);
      return new (ast_context_)
          ast::labelled_statement(label, switch_statement);
    }
    }
  }
  }

  __builtin_trap();
}

/* control transfer statement constructor */
ast::statement *analyzer::break_statement(std::u32string_view label) {
  return new (ast_context_) ast::break_statement(label);
}

ast::statement *analyzer::continue_statement(std::u32string_view label) {
  return new (ast_context_) ast::continue_statement(label);
}

ast::statement *analyzer::fallthrough_statement() {
  return new (ast_context_) ast::fallthrough_statement();
}

ast::statement *
analyzer::return_statement(location return_location, ast::expression *value) {
  if (not scope_.is<scope::type::function>()) {
    diagnose(return_location, diagnostic::err_return_invalid_outside_of_a_func);
    return nullptr;
  }

  return new (ast_context_) ast::return_statement(value);
}

ast::statement *
analyzer::defer_statement(location, ast::statement *body) {
  return new (ast_context_) ast::defer_statement(body);
}

ast::statement *analyzer::repeat_while_statement(ast::statement *body,
                                                 ast::expression *condition) {
  return new (ast_context_) ast::repeat_while_statement(body, condition);
}

ast::statement *
analyzer::line_control_statement(std::u32string_view file_name,
                                 location line_number_location,
                                 std::u32string_view line_number) {
  llvm::APSInt value(64);
  get_integer_value(line_number, 10, value);
  if (value.getExtValue() <= 0) {
    diagnose(line_number_location,
             diagnostic::err_the_line_number_needs_to_be_greater_than_zero);
    return nullptr;
  }
  return new (ast_context_)
      ast::line_control_statement(file_name, value.getExtValue());
}

/* auxiliary constructors */

pattern *analyzer::pattern_any() {
  return new (ast_context_) ast::pattern_any();
}

pattern *analyzer::pattern_expression(ast::expression *expression) {
  return new (ast_context_) ast::pattern_expression(expression);
}

pattern *analyzer::pattern_named(std::u32string_view name, bool implicit) {
  return new (ast_context_) ast::pattern_named(name, implicit);
}

ast::pattern *
analyzer::pattern_tuple(const std::vector<ast::pattern *> &elements) {
  return new (ast_context_) ast::pattern_tuple(elements);
}

ast::pattern *analyzer::pattern_typed(ast::pattern *pattern, ast::type *type) {
  return new (ast_context_) ast::pattern_typed(pattern, type);
}

ast::pattern *analyzer::pattern_var(ast::pattern *pattern) {
  return new (ast_context_) ast::pattern_var(pattern);
}

ast::type *
analyzer::type_identifier(const std::vector<std::u32string_view> &components) {
  return new (ast_context_) ast::type_identifier(components);
}

ast::type *analyzer::type_tuple(const std::vector<ast::type *> &elements) {
  return new (ast_context_) ast::type_tuple(elements);
}

ast::type *analyzer::type_array(ast::type *type) {
  return new (ast_context_) ast::type_array(type);
}

ast::type *
analyzer::type_dictionary(ast::type *key_type, ast::type *value_type) {
  return new (ast_context_) ast::type_dictionary(key_type, value_type);
}

ast::type *
analyzer::type_function(ast::type *parameter_type, ast::type *return_type) {
  return new (ast_context_) ast::type_function(parameter_type, return_type);
}

ast::type *analyzer::type_metatype(ast::type *subtype) {
  return new (ast_context_) ast::type_metatype(subtype);
}

ast::type *
analyzer::type_composite(const std::vector<ast::type_identifier *> &protocols) {
  return new (ast_context_) ast::type_composite(protocols);
}

ast::type *analyzer::type_inout(ast::type *type) {
  return new (ast_context_) ast::type_inout(type);
}

ast::statement *analyzer::build_configuration_block(ast::statement * condition,
                                                    ast::statement *
                                                        true_clause,
                                                    ast::statement *
                                                        false_clause) {
  return new (ast_context_)
      ast::build_configuration_statement(condition, true_clause, false_clause);
}
}

