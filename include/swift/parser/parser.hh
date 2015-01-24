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

#ifndef swift_parser_parser_hh
#define swift_parser_parser_hh

#include "swift/diagnostics/engine.hh"
#include "swift/parser/result.hh"
#include "swift/semantic/analyzer.hh"

#include <array>
#include <vector>

namespace swift {
class lexer;

namespace ast {
class declaration;
class expression;
class pattern;
class statement;
class type;
}

template <swift::token::type...>
struct set;

template <>
struct set<> {
  static constexpr bool contains(swift::token::type) {
    return false;
  }
};

template <swift::token::type Type, swift::token::type... Types>
struct set<Type, Types...> {
  static constexpr bool contains(swift::token::type type) {
    return type == Type or set<Types...>::contains(type);
  }
};

class parser {
  lexer &lexer_;
  semantic::analyzer &semantic_analyzer_;
  diagnostics::engine &diagnostics_engine_;

  void consume_until(token::type);

  template <swift::token::type... Types>
  void consume_until(const set<Types...> &);

  parse::result<ast::statement> parse_statements();
  parse::result<ast::statement> parse_statement();

  parse::result<ast::expression> parse_expression();
  parse::result<ast::expression> parse_prefix_expression();
  parse::result<ast::expression> parse_in_out_expression();
  parse::result<ast::expression> parse_prefix_operator();
  parse::result<ast::expression> parse_operator();
  parse::result<ast::expression> parse_postfix_expression();
  parse::result<ast::expression> parse_function_call_expression(ast::expression *function);
  parse::result<ast::expression> parse_subscript_expression(ast::expression *expression);
  parse::result<ast::expression> parse_trailing_closure();
  parse::result<ast::expression> parse_literal_expression();
  parse::result<ast::expression> parse_self_expression();
  parse::result<ast::expression> parse_superclass_expression();
  parse::result<ast::expression> parse_closure_expression();
  parse::result<ast::expression> parse_parenthesized_expression();
  parse::result<ast::expression> parse_implicit_member_expression();
  parse::result<ast::expression> parse_wildcard_expression();
  parse::result<ast::expression> parse_primary_expression();
  parse::result<ast::expression> parse_postfix_operator();
  parse::result<ast::expression> parse_initializer_expression(ast::expression *expression);
  parse::result<ast::expression> parse_explicit_member_expression(ast::expression *expression);
  parse::result<ast::expression> parse_postfix_self_expression(ast::expression *expression);
  parse::result<ast::expression> parse_dynamic_type_expression(ast::expression *expression);
  parse::result<ast::expression> parse_forced_value_expression(ast::expression *expression);
  parse::result<ast::expression> parse_optional_chaining_expression(ast::expression *expression);
  parse::result<ast::expression> parse_binary_expressions(ast::expression *lhs);
  std::array<parse::result<ast::expression>, 2> parse_binary_expression();
  std::array<parse::result<ast::expression>, 2> parse_type_casting_operator();
  parse::result<ast::expression> parse_binary_operator();
  parse::result<ast::expression> parse_assignment_operator();
  parse::result<ast::expression> parse_conditional_operator();

  // Statements
  parse::result<ast::statement> parse_loop_statement();
  parse::result<ast::statement> parse_for_statement();
  parse::result<ast::statement> parse_for_in_statement();
  parse::result<ast::statement> parse_while_statement();
  parse::result<ast::statement> parse_repeat_while_statement();
  parse::result<ast::statement> parse_while_condition();

  parse::result<ast::statement> parse_branch_statement();
  parse::result<ast::statement> parse_if_statement();
  parse::result<ast::statement> parse_switch_statement();

  parse::result<ast::statement> parse_labelled_statement();

  parse::result<ast::statement> parse_control_transfer_statement();
  parse::result<ast::statement> parse_break_statement();
  parse::result<ast::statement> parse_continue_statement();
  parse::result<ast::statement> parse_fallthrough_statement();
  parse::result<ast::statement> parse_return_statement();

  parse::result<ast::statement> parse_defer_statement();

  parse::result<ast::statement> parse_do_statement();

  parse::result<ast::statement> parse_compiler_control_statement();
  parse::result<ast::statement> parse_build_configuration_statement();
  parse::result<ast::statement> parse_line_control_statement();

  // Declarations
  parse::result<ast::statement> parse_declarations();

  parse::result<ast::declaration> parse_declaration();
  parse::result<ast::declaration> parse_import_declaration();
  parse::result<ast::declaration> parse_constant_declaration();
  parse::result<ast::declaration> parse_variable_declaration();
  parse::result<ast::declaration> parse_typealias_declaration();
  parse::result<ast::declaration> parse_function_declaration();
  parse::result<ast::declaration> parse_enum_declaration();
  parse::result<ast::declaration> parse_enum_case_name();
  parse::result<ast::declaration> parse_struct_declaration();
  parse::result<ast::declaration> parse_class_declaration();
  parse::result<ast::declaration> parse_protocol_declaration();
  parse::result<ast::declaration> parse_initializer_declaration();
  parse::result<ast::declaration> parse_deinitializer_declaration();
  parse::result<ast::declaration> parse_extension_declaration();
  parse::result<ast::declaration> parse_subscript_declaration();
  parse::result<ast::declaration> parse_operator_declaration();

  parse::result<ast::statement> parse_code_block();

  bool parse_declaration_modifiers();
  bool parse_declaration_modifier();

  bool parse_access_level_modifiers();
  bool parse_access_level_modifier();

  // Types
  parse::result<ast::type> parse_type_annotation();
  parse::result<ast::type> parse_type();
  parse::result<ast::type> parse_type_identifier();
  parse::result<ast::type> parse_tuple_type();
  parse::result<ast::type> parse_protocol_composition_type();
  bool parse_type_inheritance_clause();
  bool parse_protocol_member_declarations();
  bool parse_protocol_member_declaration();

  //// ????
  std::vector<ast::pattern *> parse_parameter_clauses();
  parse::result<ast::pattern> parse_parameter_clause();
  std::vector<ast::pattern *> parse_parameter_list();
  parse::result<ast::pattern> parse_parameter();
  parse::result<ast::type> parse_function_result();

  // Attributes
  // TODO(compnerd) push 'is_declaration' down into a sema scope?
  bool parse_attributes(bool is_declaration);
  bool parse_attribute(bool is_declaration);

  // Patterns
  parse::result<ast::pattern> parse_pattern();
  parse::result<ast::pattern> parse_value_binding_pattern();
  parse::result<ast::pattern> parse_tuple_pattern();

  /// ???
  // TODO(compnerd) should this be a statement instead?
  bool parse_pattern_initializer_list(std::vector<ast::declaration *> &list);
  parse::result<ast::declaration> parse_pattern_initializer();
  parse::result<ast::expression> parse_initializer();

  // Generic Parameters and Arguments
  bool parse_generic_parameter_clause();
  parse::result<ast::type> parse_generic_argument_clause();

  diagnostics::builder
  diagnose(location location, diagnostics::diagnostic::id id) {
    return diagnostics_engine_.report(location, id);
  }

  diagnostics::builder diagnose(range range, diagnostics::diagnostic::id id) {
    return diagnostics_engine_.report(range, id);
  }

public:
  parser(lexer &lexer, semantic::analyzer &semantic_analyzer,
         diagnostics::engine &engine)
      : lexer_(lexer), semantic_analyzer_(semantic_analyzer),
        diagnostics_engine_(engine) {}

  parse::result<ast::statement> parse_top_level_declaration();
};
}

#endif

