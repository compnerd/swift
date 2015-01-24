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

#ifndef swift_semantic_analyzer_hh
#define swift_semantic_analyzer_hh

#include "swift/diagnostics/engine.hh"
#include "swift/lexer/token.hh"
#include "swift/semantic/scope.hh"
#include "swift/syntax/switch-statement.hh"

#include <vector>
#include <ext/string_view>

namespace swift {
namespace ast {
class context;
class declaration;
class declaration_context;
class expression;
class pattern;
class statement;
class type;
class type_identifier;
}

namespace semantic {
class analyzer {
  scope scope_;
  ast::context &ast_context_;
  diagnostics::engine &diagnostics_engine_;
  ast::declaration_context *declaration_context_;

  analyzer(const analyzer &) = delete;
  analyzer &operator=(const analyzer &) = delete;

  diagnostics::builder
  diagnose(location location, diagnostics::diagnostic::id id) {
    return diagnostics_engine_.report(location, id);
  }

  diagnostics::builder diagnose(range range, diagnostics::diagnostic::id id) {
    return diagnostics_engine_.report(range, id);
  }

public:
  class declaration_context_raii {
    semantic::analyzer &semantic_analyzer_;
    ast::declaration_context *saved_context_;

  public:
    explicit declaration_context_raii(analyzer &semantic_analyzer,
                                      ast::declaration_context *current_context)
        : semantic_analyzer_(semantic_analyzer),
          saved_context_(semantic_analyzer.declaration_context_) {
      assert(current_context && "null declaration context");
      semantic_analyzer_.declaration_context_ = current_context;
    }

    ~declaration_context_raii() {
      reset();
    }

    void reset() {
      if (saved_context_)
        semantic_analyzer_.declaration_context_ = saved_context_;
      saved_context_ = nullptr;
    }
  };

  analyzer(ast::context &ast_context);

  const scope &current_scope() const noexcept {
    return scope_;
  }
  scope &current_scope() noexcept {
    return scope_;
  }

  ast::context &ast_context() {
    return ast_context_;
  }

  /* expression constructors */
  ast::expression *prefix_unary_expression(ast::expression *prefix_operator,
                                           ast::expression *subexpression);

  ast::expression *in_out_expression(ast::expression *subexpression);

  ast::expression *
  sequence_expression(const std::vector<ast::expression *> &expressions);

  ast::expression *postfix_unary_expression(ast::expression *subexpression,
                                            ast::expression *postfix_operator);

  ast::expression *
  assignment_expression(ast::expression *lhs, ast::expression *rhs);

  ast::expression *conditional_expression(ast::expression *condition,
                                          ast::expression *if_true,
                                          ast::expression *if_false);

  ast::expression *is_subtype_expression();

  ast::expression *checked_cast_expression();

  ast::expression *conditional_checked_cast_expression();

  ast::expression *
  function_call(ast::expression *function, ast::expression *arguments);

  ast::expression *initializer_expression(ast::expression *declaration);

  ast::expression *explicit_member_expression(ast::expression *expression,
                                              std::u32string_view member);

  ast::expression *postfix_self_expression(ast::expression *instance);

  ast::expression *dynamic_type_expression(ast::expression *expression);

  ast::expression *subscript_expression();

  ast::expression *forced_value_expression(ast::expression *expression);

  ast::expression *optional_chaining_expression();

  ast::expression *declaration_reference_expression(std::u32string_view value);

  ast::expression *boolean_literal_expression(const token &token);

  ast::expression *floating_point_literal_expression(std::u32string_view value);

  ast::expression *integer_literal_expression(std::u32string_view literal);

  ast::expression *nil_literal_expression();

  ast::expression *string_literal_expression(std::u32string_view literal);

  ast::expression *array_literal_expression(ast::expression *items);

  ast::expression *dictionary_literal_expression(ast::expression *items);

  ast::expression *magic_literal_expression(const token &token);

  ast::expression *superclass_expression();

  ast::expression *closure_expression(ast::statement *statements);

  ast::expression *
  parenthesized_expression(const std::vector<ast::expression *> &elements);

  ast::expression *implicit_member_expression(std::u32string_view name);

  ast::expression *wildcard_expression();


  /* declaration constructors */
  ast::declaration *
  import_declaration(range declaration_range, std::u32string_view import_path);

  ast::declaration *
  constant_declaration(ast::pattern *name, ast::expression *initializer);

  ast::declaration *
  variable_declaration(ast::pattern *name, ast::expression *initializer);

  ast::declaration *
  typealias_declaration(std::u32string_view alias, std::u32string_view type);

  ast::declaration *
  function_declaration(std::u32string_view functionname,
                       const std::vector<ast::pattern *> &parameters,
                       ast::type *result, ast::statement *body);

  ast::declaration *
  enum_declaration(std::u32string_view enumeration_name,
                   const std::vector<ast::declaration *> &elements);

  ast::declaration *struct_declaration(std::u32string_view struct_name,
                                       ast::statement *declarations);

  ast::declaration *class_declaration(std::u32string_view class_name,
                                      ast::statement *statements);

  ast::declaration *protocol_declaration(std::u32string_view protocol_name,
                                         ast::statement *members);

  ast::declaration *
  initializer_declaration(ast::pattern *parameters, ast::statement *body);

  ast::declaration *deinitializer_declaration(ast::statement *body);

  ast::declaration *extension_declaration(std::u32string_view type_name,
                                          const std::vector<std::u32string_view> &adopted_protocols,
                                          ast::statement *body);

  ast::declaration *
  subscript_declaration(ast::pattern *parameters, ast::type *return_type,
                        ast::declaration *getter, ast::declaration *setter);

  ast::declaration *
  operator_declaration(token type, std::u32string_view operator_name,
                       uint8_t precedence, token associativity);


  /* statement constructors */
  ast::statement *statements(std::vector<ast::statement *> &statements);


  /* loop statement constructors */
  ast::statement *
  for_statement(ast::statement *initializer, ast::expression *condition,
                ast::expression *increment, ast::statement *body);

  ast::statement *for_in_statement(ast::pattern *item,
                                   ast::expression *collection,
                                   ast::statement *statements);

  ast::statement *
  while_statement(ast::statement *condition, ast::statement *body);

  /* branch statement constructors */
  ast::statement *if_statement(ast::statement *condition,
                               ast::statement *true_clause,
                               ast::statement *false_clause);

  ast::statement *
  switch_statement(ast::expression *control_expression,
                   std::vector<ast::switch_statement::case_item> &case_statements);

  /* labelled statement constructor */
  ast::statement *labelled_statement(std::u32string_view label,
                                     ast::statement *loop_or_switch_statement);


  /* control transfer statement constructors */
  ast::statement *break_statement(std::u32string_view label);

  ast::statement *continue_statement(std::u32string_view label);

  ast::statement *fallthrough_statement();

  ast::statement *return_statement(location return_location,
                                   ast::expression *value);

  ast::statement *
  defer_statement(location defer_location, ast::statement *body);

  ast::statement *
  repeat_while_statement(ast::statement *body, ast::expression *condition);

  ast::statement *line_control_statement(std::u32string_view file_name,
                                         location line_number_location,
                                         std::u32string_view line_number);

  /* auxiliary constructors */
  ast::declaration *enumeration_element_declaration(std::u32string_view name);

  /* pattern constructors */
  ast::pattern *pattern_any();
  ast::pattern *pattern_expression(ast::expression *expression);
  ast::pattern *pattern_named(std::u32string_view name, bool implicit);
  ast::pattern *pattern_tuple(const std::vector<ast::pattern *> &elements);
  ast::pattern *pattern_typed(ast::pattern *pattern, ast::type *type);
  ast::pattern *pattern_var(ast::pattern *pattern);

  // Type Constructor
  ast::type *
  type_identifier(const std::vector<std::u32string_view> &components);
  ast::type *type_tuple(const std::vector<ast::type *> &elements);
  ast::type *type_array(ast::type *type);
  ast::type *type_dictionary(ast::type *key_type, ast::type *value_type);
  ast::type *type_function(ast::type *parameter_type, ast::type *return_type);
  ast::type *type_metatype(ast::type *subtype);
  ast::type *
  type_composite(const std::vector<ast::type_identifier *> &protocols);
  ast::type *type_inout(ast::type *type);

  ast::statement *build_configuration_block(ast::statement *condition,
                                            ast::statement *true_clause,
                                            ast::statement *false_clause);
};
}
}

#endif

