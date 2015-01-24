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

#include "swift/lexer/lexer.hh"
#include "swift/parser/parser.hh"
#include "swift/semantic/import_kind.hh"
#include "swift/support/error-handling.hh"
#include "swift/support/tribool.hh"
#include "swift/syntax/statements.hh"
#include "swift/syntax/declaration.hh"
#include "swift/syntax/expression.hh"
#include "swift/syntax/literal-expression.hh"
#include "swift/syntax/pattern.hh"
#include "swift/syntax/pattern-tuple.hh"

using namespace swift::diagnostics;

static constexpr bool ellipsis(const swift::token &token) {
  return token.is<swift::token::type::op>() and token.value() == U"...";
}
static constexpr bool less(const swift::token &token) {
  return token.is<swift::token::type::op>() and token.value() == U"<";
}
static constexpr bool greater(const swift::token &token) {
  return token.is<swift::token::type::op>() and token.value() == U">";
}
static constexpr bool underscore(const swift::token &token) {
  return token.is<swift::token::type::identifier>() and token.value() == U"_";
}

namespace swift {
void parser::consume_until(token::type type) {
  while (not (lexer_.head() == type or lexer_.head().is<token::type::eof>()))
    lexer_.next();
}

template <token::type... Types>
void parser::consume_until(const set<Types...> &types) {
  while (not (types.contains(lexer_.head()) or
              lexer_.head().is<token::type::eof>()))
    lexer_.next();
}

// top-level-declaration → statements[opt]
parse::result<ast::statement> parser::parse_top_level_declaration() {
  if (lexer_.head() == token::type::eof)
    return parse::result<ast::statement>();

  parse::result<ast::statement> statement = parse_statement();
  if (ast::isa<ast::statement::type::expression>(statement)) {
    std::vector<ast::statement *> statements{ *statement };
    statement = semantic_analyzer_.statements(statements);
  }
  return statement;
}

// statements → statement statements[opt]
parse::result<ast::statement> parser::parse_statements() {
  parse::result<ast::statement> statements;
  std::vector<ast::statement *> parsed_statements;

  for (bool parsed = false; ; parsed = true)
    if (auto statement = parse_statement())
      parsed_statements.push_back(*statement);
    else
      break;

  if (parsed_statements.size() == 1 and
      not semantic_analyzer_.current_scope()
              .is<semantic::scope::type::switch_statement>())
    return (statements = *parsed_statements.begin());
  else
    return (statements = semantic_analyzer_.statements(parsed_statements));
}

// statement → expression ';'[opt]
// statement → declaration ';'[opt]
// statement → loop-statement ';'[opt]
// statement → branch-statement ';'[opt]
// statement → labeled-statement ';'[opt]
// statement → control-transfer-statement ';'[opt]
// statement → defer-statement ';'[opt]
// statement → do-statement ';'[opt]
// statement → compiler-control-statement
parse::result<ast::statement> parser::parse_statement() {
  parse::result<ast::statement> statement;

  switch (lexer_.head()) {
  case token::type::identifier:
    if (lexer_.peek().is<token::type::colon>()) {
      if ((statement = parse_labelled_statement()))
        break;
      return statement;
    }
    [[clang::fallthrough]];
  default:
    if ((statement = parse_expression()))
      break;

    switch (lexer_.head()) {
    default:
      /* invalid expression statement -- diagnose */
      return statement;
    case token::type::r_brace:
      break;
    case token::type::kw_operator:
      diagnose(lexer_.head().location(),
               diagnostic::err_operator_must_be_declared_as_prefix_postfix_or_infix);
      return statement;
    }
  case token::type::at:  // attributes
  case token::type::kw_import:
  case token::type::kw_let:
  case token::type::kw_var:
  case token::type::kw_typealias:
  case token::type::kw_func:
  case token::type::kw_enum:
  case token::type::kw_struct:
  case token::type::kw_class:
  case token::type::kw_protocol:
  case token::type::kw_init:
  case token::type::kw_deinit:
  case token::type::kw_extension:
  case token::type::kw_subscript:
  case token::type::kw_prefix:
  case token::type::kw_postfix:
  case token::type::kw_infix:
    if ((statement = parse_declaration()))
      break;
    return statement;
  case token::type::kw_for:
  case token::type::kw_while:
  case token::type::kw_repeat:
    if ((statement = parse_loop_statement()))
      break;
    return statement;
  case token::type::kw_if:
  case token::type::kw_switch:
    if ((statement = parse_branch_statement()))
      break;
    return statement;
  case token::type::kw_break:
  case token::type::kw_continue:
  case token::type::kw_fallthrough:
  case token::type::kw_return:
    if ((statement = parse_control_transfer_statement()))
      break;
    return statement;
  case token::type::pp_if:
  case token::type::pp_line:
    return parse_compiler_control_statement();
  }

  if (lexer_.head().is<token::type::semi>())
    lexer_.next();
  return statement;
}

// expression → try-operator[opt] prefix-expression binary-expressions[opt]
parse::result<ast::expression> parser::parse_expression() {
  bool push_scope =
      semantic_analyzer_.current_scope().is<semantic::scope::type::top_level>();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::expression);
  if (not push_scope)
    scope.reset();

  parse::result<ast::expression> expression;
  if (not (expression = parse_prefix_expression()))
    return expression;

  if (auto binary_expressions = parse_binary_expressions(expression))
    expression = binary_expressions;

  return expression;
}

// prefix-expression → prefix-operator[opt] postfix-expression
// prefix-expression → in-out-expression
parse::result<ast::expression> parser::parse_prefix_expression() {
  if (lexer_.head().is<token::type::amp>())
    return parse_in_out_expression();

  parse::result<ast::expression> prefix_operator;
  parse::result<ast::expression> postfix_expression;

  if (lexer_.head().is<token::operator_type::unary_prefix>())
    if (not (prefix_operator = parse_prefix_operator()))
      return prefix_operator;

  postfix_expression = parse_postfix_expression();
  if (prefix_operator)
    postfix_expression =
        semantic_analyzer_.prefix_unary_expression(prefix_operator,
                                                   postfix_expression);
  return postfix_expression;
}

// in-out-expression → '&' identifier
parse::result<ast::expression> parser::parse_in_out_expression() {
  assert(lexer_.head().is<token::type::amp>() && "expected '&'");

  parse::result<ast::expression> in_out_expression;
  parse::result<ast::expression> expression;

  lexer_.next().location().start();

  location expression_location = lexer_.head().location().start();
  if (not (expression = parse_expression())) {
    diagnose(expression_location, diagnostic::err_expected_expression);
    return in_out_expression;
  }

  return (in_out_expression = semantic_analyzer_.in_out_expression(expression));
}

// prefix-operator → operator
parse::result<ast::expression> parser::parse_prefix_operator() {
  parse::result<ast::expression> prefix_operator;

  if (not lexer_.head().is<token::operator_type::unary_prefix>())
    return prefix_operator;

  token op = lexer_.next();
  return (prefix_operator =
              semantic_analyzer_.declaration_reference_expression(op.value()));
}

// operator → operator-head operator-characters[opt]
// operator → dot-operator-head dot-operator-characters[opt]
parse::result<ast::expression> parser::parse_operator() {
  assert(lexer_.head().is<token::type::op>() && "expected operator");
  __builtin_trap();
}

// postfix-expression → primary-expression
// postfix-expression → postfix-expression postfix-operator
// postfix-expression → function-call-expression
// postfix-expression → initializer-expression
// postfix-expression → explicit-member-expression
// postfix-expression → postfix-self-expression
// postfix-expression → dynamic-type-expression
// postfix-expression → subscript-expression
// postfix-expression → forced-value-expression
// postfix-expression → optional-chaining-expression
parse::result<ast::expression> parser::parse_postfix_expression() {
  parse::result<ast::expression> postfix_expression;

  postfix_expression = parse_primary_expression();

  while (true) {
    switch (lexer_.head()) {
    default:
      if (lexer_.head().is<token::type::op>() and
          lexer_.head().is<token::operator_type::unary_postfix>()) {
        parse::result<ast::expression> postfix_operator =
            parse_postfix_operator();

        postfix_expression =
            semantic_analyzer_.postfix_unary_expression(postfix_expression,
                                                        postfix_operator);
      }

      return postfix_expression;

    case token::type::l_paren:
      postfix_expression = parse_function_call_expression(postfix_expression);
      break;
    case token::type::period: {
      token period = lexer_.next();

      switch (lexer_.head()) {
      default:
        diagnose(lexer_.head().location().start(),
                 diagnostic::err_expected_member_name_following_token)
            << period;
        return parse::result<ast::expression>();
      case token::type::kw_init:
        postfix_expression = parse_initializer_expression(postfix_expression);
        break;
      case token::type::literal:
      case token::type::identifier:
        // analyze this here to avoid passing the reference to the period
        if (lexer_.head().is<token::type::identifier>() and
            lexer_.head().is<token::literal_type::string>()) {
          diagnose(lexer_.head().location(),
                   diagnostic::err_expected_member_name_following_token)
              << period;
          return parse::result<ast::expression>();
        }

        postfix_expression =
            parse_explicit_member_expression(postfix_expression);
        break;
      case token::type::kw_self:
        postfix_expression = parse_postfix_self_expression(postfix_expression);
        break;
      case token::type::kw_dynamicType:
        postfix_expression = parse_dynamic_type_expression(postfix_expression);
        break;
      }

      break;
    }
    case token::type::l_square:
      postfix_expression = parse_subscript_expression(postfix_expression);
      break;
    case token::type::exclaim:  // forced-value-expression
      if (not lexer_.head().is<token::operator_type::unary_postfix>())
        return parse::result<ast::expression>();
      postfix_expression = parse_forced_value_expression(postfix_expression);
      break;
    case token::type::question:
      if (not lexer_.head().is<token::operator_type::unary_postfix>())
        return parse::result<ast::expression>();
      postfix_expression =
          parse_optional_chaining_expression(postfix_expression);
      break;
    }

    if (not postfix_expression)
      return postfix_expression;
  }

  swift_unreachable("should have produced an AST or diagnostic");
}

parse::result<ast::expression>
parser::parse_initializer_expression(ast::expression *expression) {
  assert(lexer_.head().is<token::type::kw_init>() && "expected 'init'");

  parse::result<ast::expression> initializer_expression;
  token init = lexer_.next();

  if (not lexer_.peek().is<token::type::l_paren>()) {
    diagnose(init.location(),
             diagnostic::err_initializer_cannot_be_referenced_without_arguments);
    return initializer_expression;
  }

  initializer_expression =
      semantic_analyzer_.initializer_expression(expression);
  return initializer_expression;
}

parse::result<ast::expression>
parser::parse_explicit_member_expression(ast::expression *expression) {
  assert((lexer_.head().is<token::type::identifier>() or
          lexer_.head().is<token::type::literal>()) &&
         "expected identifier or literal");

  parse::result<ast::expression> explicit_member_expression;
  explicit_member_expression =
      semantic_analyzer_.explicit_member_expression(expression,
                                                    lexer_.next().value());

  if (less(lexer_.head()))
    parse_generic_argument_clause();

  return explicit_member_expression;
}

parse::result<ast::expression>
parser::parse_postfix_self_expression(ast::expression *expression) {
  assert(lexer_.head().is<token::type::kw_self>() && "expected 'self'");
  lexer_.next();
  return parse::result<ast::expression>(
      semantic_analyzer_.postfix_self_expression(expression));
}

parse::result<ast::expression>
parser::parse_dynamic_type_expression(ast::expression *expression) {
  assert(lexer_.head().is<token::type::kw_dynamicType>() &&
         "expected 'dynamicType'");
  lexer_.next();
  return parse::result<ast::expression>(
      semantic_analyzer_.dynamic_type_expression(expression));
}

parse::result<ast::expression>
parser::parse_forced_value_expression(ast::expression *expression) {
  assert((lexer_.head().is<token::type::exclaim>() and
          lexer_.head().is<token::operator_type::unary_postfix>()) &&
         "expected unary postfix '!'");
  lexer_.next();
  return parse::result<ast::expression>(
      semantic_analyzer_.forced_value_expression(expression));
}

parse::result<ast::expression>
parser::parse_optional_chaining_expression(ast::expression *) {
  assert((lexer_.head().is<token::type::question>() and
          lexer_.head().is<token::operator_type::unary_postfix>()) &&
         "expected postfix unary '?'");
  __builtin_trap();
}

// function-call-expression → postfix-expression parenthesized-expression
// function-call-expression → postfix-expression parenthesized-expression[opt] trailing-closure
parse::result<ast::expression>
parser::parse_function_call_expression(ast::expression *function) {
  assert((lexer_.head().is<token::type::l_paren>() or
          lexer_.head().is<token::type::l_brace>()) &&
         "expected postfix-expression to be parsed");

  parse::result<ast::expression> function_call;

  bool push_scope =
      semantic_analyzer_.current_scope().is<semantic::scope::type::top_level>();
  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::statement);
  if (not push_scope)
    scope.reset();

  if (lexer_.head().is<token::type::l_brace>())
    __builtin_trap(); // return parse_trailing_closure();

  parse::result<ast::expression> arguments = parse_parenthesized_expression();
  if (not arguments)
    return function_call;

  if (not semantic_analyzer_.current_scope().is<semantic::scope::type::branch>())
    if (lexer_.head().is<token::type::l_brace>())
      parse_trailing_closure();

  return (function_call = semantic_analyzer_.function_call(function, arguments));
}

// generic-parameter-clause → '<' generic-parameter-list requirement-clause[opt] '>'
//
// generic-parameter-list → generic-parameter
// generic-parameter-list → generic-parameter ',' generic-parameter-list
//
// generic-parameter → type-name
// generic-parameter → type-name ':' type-identifier
// generic-parameter → type-name ':' protocol-composition-type
//
// requirement-clause → 'where' requirement-list
//
// requirement-list → requirement
// requirement-list → requirement ',' requirement-list
//
// requirement → conformance-requirement
// requirement → same-type-requirement
//
// conformance-requirement → type-identifier ':' type-identifier
// conformance-requirement → type-identifier ':' protocol-composition-type
//
// same-type-requirement → type-identifier '==' type-identifier
bool parser::parse_generic_parameter_clause() {
  assert(less(lexer_.head()) && "expected '<'");
  __builtin_trap();
}

// generic-argument-clause → '<' generic-argument-list '>'
// generic-argument-list → generic-argument
// generic-argument-list → generic-argument ',' generic-argument-list
// generic-argument → type
parse::result<ast::type> parser::parse_generic_argument_clause() {
  assert(less(lexer_.head()) && "expected '<'");

  parse::result<ast::type> type;

  token less = lexer_.next();
  for (; not lexer_.head().is<token::type::eof>(); ) {
    location type_location = lexer_.head().location().start();

    if (not (type = parse_type())) {
      diagnose(type_location, diagnostic::err_expected_type);
      return type;
    }

    if (not lexer_.head().is<token::type::comma>())
      break;
    lexer_.next();
  }

  if (not greater(lexer_.head())) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_to_syntax)
        << token(token::type::op, U">", location(), location())
        << "complete generic argument list";
    diagnose(less.location().start(),
             diagnostic::note_to_match_this_opening_token)
        << less;
    return type;
  }
  lexer_.next();

  return type;
}

// subscript-expression → postfix-expression '[' expression-list ']'
parse::result<ast::expression>
parser::parse_subscript_expression(ast::expression *) {
  __builtin_trap();
}

// trailing-closure → closure-expression
parse::result<ast::expression> parser::parse_trailing_closure() {
  assert(lexer_.head().is<token::type::l_brace>() && "expected '{'");

  assert(not semantic_analyzer_.current_scope()
                 .is<semantic::scope::type::branch>() &&
         "should not be in a branch scope");

  return parse_closure_expression();
}

// primary-expression → identifier generic-argument-clause[opt]
// primary-expression → literal-expression
// primary-expression → self-expression
// primary-expression → superclass-expression
// primary-expression → closure-expression
// primary-expression → parenthesized-expression
// primary-expression → implicit-member-expression
// primary-expression → wildcard-expression
parse::result<ast::expression> parser::parse_primary_expression() {
  parse::result<ast::expression> primary_expression;

  switch (lexer_.head()) {
  default: break;

  case token::type::identifier: {
    token identifier = lexer_.next();

    if (less(lexer_.head()))
      parse_generic_argument_clause();

    primary_expression =
        semantic_analyzer_.declaration_reference_expression(identifier.value());
    break;
  }
  case token::type::literal:
  case token::type::l_square:
  case token::type::kw___FILE__:
  case token::type::kw___LINE__:
  case token::type::kw___COLUMN__:
  case token::type::kw___FUNCTION__:
    return parse_literal_expression();
  case token::type::kw_self:
    return parse_self_expression();
  case token::type::kw_super:
    return parse_superclass_expression();
  case token::type::l_brace:
    return parse_closure_expression();
  case token::type::l_paren:
    return parse_parenthesized_expression();
  case token::type::period:
    return parse_implicit_member_expression();
  case token::type::underscore:
    return parse_wildcard_expression();
  }

  return primary_expression;
}

// postfix-operator → operator
parse::result<ast::expression> parser::parse_postfix_operator() {
  parse::result<ast::expression> postfix_operator;

  if (not lexer_.head().is<token::operator_type::unary_postfix>())
    return postfix_operator;

  token op = lexer_.next();
  return (postfix_operator =
              semantic_analyzer_.declaration_reference_expression(op.value()));
}

// literal-expression → literal
// literal-expression → array-literal
// literal-expression → dictionary-literal
//
// literal-expression → __FILE__
// literal-expression → __LINE__
// literal-expression → __COLUMN__
// literal-expression → __FUNCTION__
//
// array-literal → '[' array-literal-items[opt] ']'
// array-literal-items → array-literal-item ','[opt]
// array-literal-items → array-literal-item ',' array-literal-items
// array-literal-item → expression
//
// dictionary-literal → '[' dictionary-literal-items ']'
// dictionary-literal → '[' ':' ']'
// dictionary-literal-items → dictionary-literal-item ','[opt]
// dictionary-literal-items → dictionary-literal-item ',' dictionary-literal-items
// dictionary-literal-item → expression ':' expression
parse::result<ast::expression> parser::parse_literal_expression() {
  assert((lexer_.head().is<token::type::literal>() or
          lexer_.head().is<token::type::l_square>() or
          lexer_.head().is<token::type::kw___COLUMN__>() or
          lexer_.head().is<token::type::kw___FILE__>() or
          lexer_.head().is<token::type::kw___FUNCTION__>() or
          lexer_.head().is<token::type::kw___LINE__>()) &&
         "expected identifier, '[', '__FILE__', '__LINE__', '__COLUMN__', or "
         "'__FUNCTION__'");

  parse::result<ast::expression> literal_expression;

  switch (lexer_.head()) {
  default:
    swift_unreachable("invalid literal expression");
  case token::type::literal:
    switch (static_cast<token::literal_type>(lexer_.head())) {
    case token::literal_type::invalid:
      swift_unreachable("invalid literal type");
    case token::literal_type::integral:
      literal_expression =
          semantic_analyzer_.integer_literal_expression(lexer_.next().value());
      break;
    case token::literal_type::floating_point:
      literal_expression =
          semantic_analyzer_
              .floating_point_literal_expression(lexer_.next().value());
      break;
    case token::literal_type::string:
      literal_expression =
          semantic_analyzer_.string_literal_expression(lexer_.next().value());
      break;
    case token::literal_type::constant_true:
    case token::literal_type::constant_false:
      literal_expression =
          semantic_analyzer_.boolean_literal_expression(lexer_.next());
      break;
    case token::literal_type::constant_nil:
      lexer_.next();
      literal_expression = semantic_analyzer_.nil_literal_expression();
      break;
    }
    break;
  case token::type::l_square: {
    tribool is_dictionary(tribool::indeterminate);
    std::vector<ast::expression *> elements;

    token l_square = lexer_.next();

    while (true) {
      parse::result<ast::expression> key;
      parse::result<ast::expression> value;

      if (not (key = parse_expression()))
        break;

      if (indeterminate(is_dictionary))
        is_dictionary = lexer_.head().is<token::type::colon>();

      if (is_dictionary) {
        if (not lexer_.head().is<token::type::colon>()) {
          diagnose(lexer_.head().location().start(),
                   diagnostic::err_expected_token_in)
              << token(token::type::colon, U":", location(), location())
              << "dictionary literal";
          return literal_expression;
        }
        lexer_.next();

        if (not (value = parse_expression())) {
          diagnose(lexer_.head().location().start(),
                   diagnostic::err_expected_value_in)
              << "dictionary literal";
          return literal_expression;
        }
      }

      if (is_dictionary)
        elements.push_back(semantic_analyzer_.parenthesized_expression({ key, value }));
      else
        elements.push_back(key);

      if (not lexer_.head().is<token::type::comma>())
        break;
      lexer_.next();
    }

    if (not lexer_.head().is<token::type::r_square>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_token_in)
          << token(token::type::r_square, U"]", location(), location())
          << "container literal expression";
      diagnose(l_square.location().start(),
               diagnostic::note_to_match_this_opening_token)
          << l_square;
      return literal_expression;
    }
    lexer_.next();

    literal_expression = semantic_analyzer_.parenthesized_expression(elements);
    literal_expression =
        is_dictionary
            ? semantic_analyzer_.dictionary_literal_expression(literal_expression)
            : semantic_analyzer_.array_literal_expression(literal_expression);
    break;
  }
  case token::type::kw___FILE__:
  case token::type::kw___LINE__:
  case token::type::kw___COLUMN__:
  case token::type::kw___FUNCTION__:
    literal_expression =
        semantic_analyzer_.magic_literal_expression(lexer_.next());
    break;
  }

  return literal_expression;
}

// self-expression → 'self'
// self-expression → 'self' '.' identifier
// self-expression → 'self' '[' expression ']'
// self-expression → 'self' '.' 'init'
parse::result<ast::expression> parser::parse_self_expression() {
  assert(lexer_.head().is<token::type::kw_self>() && "expected 'self'");

  parse::result<ast::expression> self_expression;

  token self = lexer_.next();
  self_expression =
      semantic_analyzer_.declaration_reference_expression(self.value());

  switch (lexer_.head()) {
  default: break;
  case token::type::period: {
    token period = lexer_.next();

    if (not lexer_.head().is<token::type::identifier>() and
        not lexer_.head().is<token::type::kw_init>() and
        not lexer_.head().is<token::type::literal>()) {
      diagnose(lexer_.head().location(),
               diagnostic::err_expected_member_name_following_token)
          << period;
      return parse::result<ast::expression>();
    }

    self_expression =
        semantic_analyzer_.explicit_member_expression(self_expression,
                                                      lexer_.next().value());
    break;
  }
  case token::type::l_square:
    __builtin_trap();
  }

  return self_expression;
}

// superclass-expression → superclass-method-expression
// superclass-expression → superclass-subscript-expression
// superclass-expression → superclass-initializer-expression
//
// superclass-method-expression → 'super' '.' identifier
// superclass-subscript-expression → 'super' '[' expression ']'
// superclass-initializer-expression → 'super' '.' 'init'
parse::result<ast::expression> parser::parse_superclass_expression() {
  assert(lexer_.head().is<token::type::kw_super>() && "expected 'super'");

  parse::result<ast::expression> superclass_expression;

  token super = lexer_.next();
  superclass_expression = semantic_analyzer_.superclass_expression();

  switch (lexer_.head()) {
  default:
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_or_token_after_token)
        << token(token::type::period, U".", location(), location())
        << token(token::type::l_square, U"[", location(), location()) << super;
    return parse::result<ast::expression>();
  case token::type::period: {
    lexer_.next();

    if (not lexer_.head().is<token::type::identifier>() and
        not lexer_.head().is<token::type::kw_init>()) {
      diagnose(lexer_.head().location(),
               diagnostic::err_expected_identifier_or_token_after_syntax)
          << token(token::type::kw_init, U"init", location(), location())
          << "super '.' expression";
      return parse::result<ast::expression>();
    }

    superclass_expression =
        semantic_analyzer_.explicit_member_expression(superclass_expression,
                                                      lexer_.next().value());
    break;
  }
  case token::type::l_square:
    __builtin_trap();
  }

  return superclass_expression;
}

// closure-expression → '{' closure-signature[opt] statements '}'
//
// closure-signature → parameter-clause function-result[opt] 'in'
// closure-signature → identifier-list function-result[opt] 'in'
// closure-signature → capture-list parameter-clause function-result[opt] 'in'
// closure-signature → capture-list identifier-list function-result[opt] 'in'
// closure-signature → capture-list 'in'
//
// capture-list → '[' capture-specifier expression ']'
//
// capture-specifier → 'weak'
// capture-specifier → 'unowned'
// capture-specifier → 'unowned(safe)'
// capture-specifier → 'unowned(unsafe)'
//
// identifier-list → identifier
// identifier-list → identifier ',' identifier-list
parse::result<ast::expression> parser::parse_closure_expression() {
  assert(lexer_.head().is<token::type::l_brace>() && "expected '{'");

  parse::result<ast::expression> closure;
  parse::result<ast::statement> statements;

  token l_brace = lexer_.next();

  switch (lexer_.head()) {
  default: break;
  case token::type::l_paren:
    parse_parameter_clause();
    __builtin_trap();
    break;
  case token::type::l_square:
    __builtin_trap();
    break;
  case token::type::identifier:
    __builtin_trap();
  }

  if (not (statements = parse_statements()))
    __builtin_trap();

  if (not lexer_.head().is<token::type::r_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_at)
        << token(token::type::r_brace, U"}", location(), location())
        << "end of closure";
    diagnose(l_brace.location().start(),
             diagnostic::note_to_match_this_opening_token)
        << l_brace;
    return closure;
  }
  lexer_.next();

  return (closure = semantic_analyzer_.closure_expression(*statements));
}

// parenthesized-expression → '(' expression-element-list[opt] ')'
// expression-element-list → expression-element
// expression-element-list → expression-element ',' expression-element-list
// expression-element → expression
// expression-element → identifier ':' expression
parse::result<ast::expression> parser::parse_parenthesized_expression() {
  assert(lexer_.head().is<token::type::l_paren>() && "expected '('");
  lexer_.next();

  parse::result<ast::expression> parenthesized_expression;
  std::vector<ast::expression *> elements;

  while (true) {
    if (lexer_.head().is<token::type::identifier>() and
        lexer_.peek().is<token::type::colon>()) {
      assert(lexer_.head().is<token::type::identifier>() &&
             "expected identifier");
      lexer_.next();

      assert(lexer_.head().is<token::type::colon>() && "expected ':'");
      lexer_.next();

    }

    parse::result<ast::expression> element = parse_expression();
    if (not element and elements.size()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_expression_in)
          << "list of expression";
      return parenthesized_expression;
    }
    if (element)
      elements.push_back(element);

    if (not lexer_.head().is<token::type::comma>())
      break;

    if (not element) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_unexpected_token_separator)
          << token(token::type::comma, U",", location(), location());
      return parenthesized_expression;
    }

    lexer_.next();
  }

  assert(lexer_.head().is<token::type::r_paren>() && "expected ')'");
  lexer_.next();

  parenthesized_expression =
      semantic_analyzer_.parenthesized_expression(elements);
  return parenthesized_expression;
}

// implicit-member-expression → '.' identifier
parse::result<ast::expression> parser::parse_implicit_member_expression() {
  assert(lexer_.head().is<token::type::period>() && "expected '.'");

  parse::result<ast::expression> implicit_member_expression;

  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_after_syntax)
        << "'.' expression";
    return implicit_member_expression;
  }

  implicit_member_expression =
      semantic_analyzer_.implicit_member_expression(lexer_.next().value());
  return implicit_member_expression;
}

// wildcard-expression → '_'
parse::result<ast::expression> parser::parse_wildcard_expression() {
  assert(lexer_.head().is<token::type::underscore>() && "expected '_'");
  lexer_.next();
  return parse::result<ast::expression>(semantic_analyzer_.wildcard_expression());
}

// binary-expressions → binary-expression binary-expressions[opt]
parse::result<ast::expression>
parser::parse_binary_expressions(ast::expression *lhs) {
  auto is_binary_expression_head = [](swift::lexer &lexer) -> bool {
    return (lexer.head().is<token::type::op>() and
            lexer.head().is<token::operator_type::binary>()) or
           (lexer.head().is<token::type::question>() and
            lexer.head().is<token::operator_type::binary>()) or
           (lexer.head().is<token::type::exclaim>() and
            lexer.head().is<token::operator_type::binary>()) or
           lexer.head().is<token::type::equal>() or
           lexer.head().is<token::type::kw_is>() or
           lexer.head().is<token::type::kw_as>();
  };

  parse::result<ast::expression> sequence;
  std::vector<ast::expression *> expressions;

  while (is_binary_expression_head(lexer_)) {
    auto binary_expression = parse_binary_expression();
    if (not binary_expression[0] or not binary_expression[1])
      break;
    expressions.push_back(*binary_expression[0]);
    expressions.push_back(*binary_expression[1]);
  }

  if (expressions.empty())
    return sequence;

  expressions.insert(expressions.begin(), lhs);
  return (sequence = semantic_analyzer_.sequence_expression(expressions));
}

// binary-expression → binary-operator prefix-expression
// binary-expression → assignment-operator prefix-expression
// binary-expression → conditional-operator prefix-expression
// binary-expression → type-casting-operator
std::array<parse::result<ast::expression>, 2>
parser::parse_binary_expression() {
  assert(((lexer_.head().is<token::type::op>() and
           lexer_.head().is<token::operator_type::binary>()) or
          lexer_.head().is<token::type::equal>() or
          (lexer_.head().is<token::type::question>() and
           lexer_.head().is<token::operator_type::binary>()) or
          (lexer_.head().is<token::type::exclaim>() and
           lexer_.head().is<token::operator_type::binary>()) or
          lexer_.head().is<token::type::kw_as>() or
          lexer_.head().is<token::type::kw_is>()) &&
         "expected binary-operator, '=', '?', 'is', or 'as'");

  std::array<parse::result<ast::expression>, 2> binary_expression;

  switch (lexer_.head()) {
  default: swift_unreachable("invalid binary_expression");
  case token::type::kw_is:  // type-casting-operator
  case token::type::kw_as:  // type-casting-operator
    return parse_type_casting_operator();
  case token::type::equal:  { // asignment-operator
    if (not parse_assignment_operator())
      swift_unreachable("parse_assignment_operator must parse '='");

    location expression_location = lexer_.head().location().start();
    binary_expression[1] = parse_prefix_expression();
    if (not binary_expression[1]) {
      diagnose(expression_location, diagnostic::err_expected_expression_in)
          << "assignment";
      return binary_expression;
    }

    binary_expression[0] =
        semantic_analyzer_.assignment_expression(nullptr, nullptr);
    return binary_expression;
  }
  case token::type::question: {  // conditional-operator
    binary_expression[0] = parse_conditional_operator();
    assert(binary_expression[0] &&
           "expected conditional-operator AST construct");

    location expression_location = lexer_.head().location().start();
    binary_expression[1] = parse_prefix_expression();
    if (not binary_expression[1]) {
      diagnose(expression_location,
               diagnostic::err_expected_expression_after_syntax)
          << "'? ... :' in ternary operator";
      return binary_expression;
    }

    return binary_expression;
  }
  case token::type::op:
  case token::type::exclaim: {  // binary-operator
    binary_expression[0] = parse_binary_operator();
    assert(binary_expression[0] && "expected binary-operator AST construct");

    location expression_location = lexer_.head().location().start();
    binary_expression[1] = parse_prefix_expression();
    if (not binary_expression[1]) {
      diagnose(expression_location,
               diagnostic::err_expected_expression_after_operator);
      return binary_expression;
    }

    return binary_expression;
  }
  }
}

// type-casting-operator → 'is' type
// type-casting-operator → 'as' type
// type-casting-operator → 'as' '?' type
std::array<parse::result<ast::expression>, 2>
parser::parse_type_casting_operator() {
  assert((lexer_.head().is<token::type::kw_is>() or
          lexer_.head().is<token::type::kw_as>()) &&
         "expected 'is' or 'as'");

  switch (lexer_.head()) {
  default:
    swift_unreachable("invalid type casting operator");
  case token::type::kw_is:
    __builtin_trap();
  case token::type::kw_as:
    __builtin_trap();
  }
}

// binary-operator → operator
parse::result<ast::expression> parser::parse_binary_operator() {
  parse::result<ast::expression> binary_operator;

  if (not lexer_.head().is<token::operator_type::binary>())
    return binary_operator;

  token op = lexer_.next();

  return parse::result<ast::expression>(
      semantic_analyzer_.declaration_reference_expression(op.value()));
}

// assignment-operator → '='
parse::result<ast::expression> parser::parse_assignment_operator() {
  assert(lexer_.head().is<token::type::equal>() && "expected '='");

  token equal = lexer_.next();
  return parse::result<ast::expression>(
      semantic_analyzer_.declaration_reference_expression(equal.value()));
}

// conditional-operator → '?' expression ':'
parse::result<ast::expression> parser::parse_conditional_operator() {
  assert(lexer_.head().is<token::type::question>() && "expected '?'");

  parse::result<ast::expression> conditional_operator;
  parse::result<ast::expression> if_true;

  token question = lexer_.next();

  location expression_location = lexer_.head().location().start();
  if (not (if_true = parse_expression())) {
    diagnose(expression_location,
             diagnostic::err_expected_expression_after_token_in)
        << question << "ternary expression";
    return conditional_operator;
  }

  if (not lexer_.head().is<token::type::colon>()) {
    diagnose(expression_location, diagnostic::err_expected_token_after_syntax)
        << token(token::type::colon, U":", location(), location())
        << "'? ...' in ternary expression";
    return conditional_operator;
  }
  // TODO(compnerd) track this location
  lexer_.next();

  conditional_operator =
      semantic_analyzer_.conditional_expression(nullptr, if_true, nullptr);
  return conditional_operator;
}

// type-annotation → : attributes[opt] type
parse::result<ast::type> parser::parse_type_annotation() {
  assert(lexer_.head().is<token::type::colon>() && "expected ':'");

  token colon = lexer_.next();

  if (not lexer_.head().is<token::type::identifier>() and
      not lexer_.head().is<token::type::l_square>() and
      not lexer_.head().is<token::type::l_paren>() and
      not lexer_.head().is<token::type::at>() and
      not lexer_.head().is<token::type::kw_protocol>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_parameter_type_following)
        << colon;
    return parse::result<ast::type>();
  }

  parse_attributes(/*is_declaration=*/false);

  return parse_type();
}

// type → array-type
// type → dictionary-type
// type → function-type
// type → type-identifier
// type → tuple-type
// type → optional-type
// type → implicitly-unwrapped-optional-type
// type → protocol-composition-type
// type → metatype-type
//
// array-type → '[' type ']'
//
// dictionary-type → '[' type ':' type ']'
//
// function-type → type '->' type
//
// optional-type → type '?'
//
// implicitly-unwrapped-optional-type → type '!'
//
// metatype-type → type '.' 'Type'
// metatype-type → type '.' 'Protocol'
parse::result<ast::type> parser::parse_type() {
  parse::result<ast::type> type;

  switch (lexer_.head()) {
  default:
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_type);
    return type;

  case token::type::l_square: {
    bool is_dictionary = false;
    parse::result<ast::type> element_type;
    parse::result<ast::type> value_type;

    token open = lexer_.next();

    location element_type_location = lexer_.head().location().start();
    if (not (element_type = parse_type())) {
      diagnose(element_type_location, diagnostic::err_expected_element_type);
      return type;
    }

    if ((is_dictionary = lexer_.head().is<token::type::colon>())) {
      lexer_.next();

      location value_type_location = lexer_.head().location().start();
      if (not (value_type = parse_type())) {
        diagnose(value_type_location,
                 diagnostic::err_expected_dictionary_value_type);
        return type;
      }
    }

    if (not lexer_.head().is<token::type::r_square>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_token_in)
          << token(token::type::r_square, U"]", location(), location())
          << (is_dictionary ? "dictionary type" : "array type");
      diagnose(open.location().start(),
               diagnostic::note_to_match_this_opening_token)
          << open;
      return type;
    }
    lexer_.next();

    type = is_dictionary
               ? semantic_analyzer_.type_dictionary(element_type, value_type)
               : semantic_analyzer_.type_array(element_type);

    break;
  }

  case token::type::identifier:
    if (not (type = parse_type_identifier()))
      return type;
    break;

  case token::type::l_paren:
    if (not (type = parse_tuple_type()))
      return type;
    break;

  case token::type::kw_protocol:
    if (not (type = parse_protocol_composition_type()))
      return type;
    break;
  }

  for (bool complete = false; not complete;) {
    switch (lexer_.head()) {
    default:
      complete = true;
      break;

    case token::type::l_square:
      if (lexer_.peek().is<token::type::r_square>()) {
        diagnose(lexer_.head().location(),
                 diagnostic::err_migration_new_array_syntax);

        // TODO(compnerd) implement consume_until(token::type);
        lexer_.next();
        lexer_.next();

        return type.invalidate();
      }

    case token::type::arrow: {
      parse::result<ast::type> result_type;

      lexer_.next();

      location type_location = lexer_.head().location().start();
      if (not (result_type = parse_type())) {
        diagnose(type_location,
                 diagnostic::err_expected_type_for_function_result);
        return type.invalidate();
      }

      type = semantic_analyzer_.type_function(type, result_type);
      break;
    }

    case token::type::question:
      diagnose(lexer_.next().location().start(),
               diagnostic::warn_unsupported_feature)
          << "optional type sugar (optional type)";
      break;

    case token::type::exclaim:
      diagnose(lexer_.next().location().start(),
               diagnostic::warn_unsupported_feature)
          << "optional type sugar (implicitly unwrapped optional type)";
      break;

    case token::type::period: {
      switch (lexer_.peek()) {
      default:
        complete = true;
        break;

      case token::type::kw_Type:
        lexer_.next();
        type = semantic_analyzer_.type_metatype(type);
        break;

      case token::type::kw_Protocol:
        __builtin_trap();
        break;
      }

      break;
    }
    }
  }

  return type;
}

// type-identifier → type-name generic-argument-clause[opt]
// type-identifier → type-name generic-argument-clause[opt] '.' type-identifier
// type-name → identifier
parse::result<ast::type> parser::parse_type_identifier() {
  parse::result<ast::type> type_identifier;
  std::vector<std::u32string_view> components;

  while (not lexer_.head().is<token::type::eof>()) {
    if (not lexer_.head().is<token::type::identifier>() and components.size()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_identifier_in)
          << "dotted type";
      return type_identifier;
    }

    components.push_back(lexer_.next().value());

    if (less(lexer_.head()))
      parse_generic_argument_clause();

    if (not lexer_.head().is<token::type::period>())
      break;
    if (lexer_.peek().is<token::type::kw_Type>() or
        lexer_.peek().is<token::type::kw_Protocol>())
      break;
    lexer_.next();
  }

  return (type_identifier = semantic_analyzer_.type_identifier(components));
}

// tuple-type → '(' tuple-type-body[opt] ')'
// tuple-type-body → tuple-type-element-list '...'[opt]
// tuple-type-element-list → tuple-type-element
// tuple-type-element-list → tuple-type-element ',' tuple-type-element-list
// tuple-type-element → attributes[opt] 'inout'[opt] type
// tuple-type-element → 'inout'[opt] element-name type-annotation
// element-name → identifier
parse::result<ast::type> parser::parse_tuple_type() {
  assert(lexer_.head().is<token::type::l_paren>() && "expected '('");

  parse::result<ast::type> type;
  std::vector<ast::type *> elements;

  lexer_.next();

  if (lexer_.head().is<token::type::r_paren>()) {
    lexer_.next();
    return (type = semantic_analyzer_.type_tuple(elements));
  }

  while (true) {
    bool has_attributes = parse_attributes(/*is_declaration=*/false);
    bool is_inout = false;

    // TODO(compnerd) check if in parameter list
    // error: 'inout' is only valid in parameter lists
    if (lexer_.head().is<token::type::kw_inout>()) {
      is_inout = true;
      lexer_.next();
    }

    bool has_type_annotation = not has_attributes and
                               lexer_.head().is<token::type::identifier>() and
                               lexer_.peek().is<token::type::colon>();

    if (not has_type_annotation) {
      parse::result<ast::type> element = parse_type();
      if (not element)
        return type;

      if (is_inout)
        element = semantic_analyzer_.type_inout(*element);
      elements.push_back(*element);
    } else {
      __builtin_trap();
    }

    if (ellipsis(lexer_.head()) and
        not lexer_.peek().is<token::type::r_paren>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_token_before_syntax)
          << lexer_.head() << "the end of a tuple list";
      return type;
    }

    if (ellipsis(lexer_.head())) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_cannot_create_variadic_tuple);
      return type;
    }

    if (not lexer_.head().is<token::type::comma>())
      break;
    lexer_.next();
  }

  if (not lexer_.head().is<token::type::r_paren>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_type)
        << token(token::type::comma, U",", location(), location())
        << "separator";
    return type;
  }
  lexer_.next();

  return (type = semantic_analyzer_.type_tuple(elements));
}

// protocol-composition-type → 'protocol' '<' protocol-identifier-list[opt] '>'
// protocol-identifier-list → protocol-identifier
// protocol-identifier-list → protocol-identifier ',' protocol-identifier-list
// protocol-identifier → type-identifier
parse::result<ast::type> parser::parse_protocol_composition_type() {
  assert(lexer_.head().is<token::type::kw_protocol>() && "expected 'protocol'");

  parse::result<ast::type> type;
  std::vector<ast::type_identifier *> protocols;

  lexer_.next();

  if (not less(lexer_.head())) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::operator_type::binary, U"<", location(), location())
        << "protocol composition type";
    return type;
  }
  token less_token = lexer_.next();

  while (true) {
    location type_identifier_location = lexer_.head().location().start();

    parse::result<ast::type> type_identifier;
    if (not (type_identifier = parse_type_identifier())) {
      diagnose(type_identifier_location,
               diagnostic::err_expected_identifier_for_type_name);
      return type;
    }

    // TODO(compnerd) get a better casting mechanism
    protocols.push_back(reinterpret_cast<ast::type_identifier *>(*type_identifier));

    if (not lexer_.head().is<token::type::comma>())
      break;
    lexer_.next();
  }

  if (not greater(lexer_.head())) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_to_syntax)
        << token(token::operator_type::binary, U">", location(), location())
        << "complete protocol composition type";
    diagnose(less_token.location().start(),
             diagnostic::note_to_match_this_opening_token)
        << less_token;
    return type;
  }
  lexer_.next();

  return (type = semantic_analyzer_.type_composite(protocols));
}

// declarations → declaration
// declarations → declaration declarations[opt]
parse::result<ast::statement> parser::parse_declarations() {
  std::vector<ast::statement *> declarations;
  parse::result<ast::statement> statement;

  for (parse::result<ast::statement> declaration;
       (declaration = parse_declaration());
       declarations.push_back(*declaration))
    if (lexer_.head().is<token::type::semi>())
      lexer_.next();

  return declarations.size()
             ? (statement = semantic_analyzer_.statements(declarations))
             : statement;
}

// declaration → import-declaration
// declaration → constant-declaration
// declaration → variable-declaration
// declaration → typealias-declaration
// declaration → function-declaration
// declaration → enum-declaration
// declaration → struct-declaration
// declaration → class-declaration
// declaration → protocol-declaration
// declaration → initializer-declaration
// declaration → deinitializer-declaration
// declaration → extension-declaration
// declaration → subscript-declaration
// declaration → operator-declaration
parse::result<ast::declaration> parser::parse_declaration() {
  bool parsed_attributes = parse_attributes(/*is_declaration=*/true);
  bool parsed_declaration_modifiers = parse_declaration_modifiers();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::declaration);
  if (lexer_.head().is<token::type::kw_import>())
    scope.reset();

  parse::result<ast::declaration> declaration;

  switch (lexer_.head()) {
  default:
    if (parsed_attributes)
      __builtin_trap();  // TODO(compnerd) diagnose invalid declaration
    return declaration;
  case token::type::kw_import:
    if (parsed_declaration_modifiers)
      __builtin_trap();  // TODO(compnerd) diagnose invalid declaration
    return parse_import_declaration();
  case token::type::kw_let:
    return parse_constant_declaration();
  case token::type::kw_var:
    return parse_variable_declaration();
  case token::type::kw_typealias:
    return parse_typealias_declaration();
  case token::type::kw_func:
    return parse_function_declaration();
  case token::type::kw_enum:
    return parse_enum_declaration();
  case token::type::kw_struct:
    return parse_struct_declaration();
  case token::type::kw_class:
    return parse_class_declaration();
  case token::type::kw_protocol:
    return parse_protocol_declaration();
  case token::type::kw_init:
    return parse_initializer_declaration();
  case token::type::kw_deinit:
    if (parsed_declaration_modifiers)
      __builtin_trap();  // TODO(compnerd) diagnose invalid declaration
    return parse_deinitializer_declaration();
  case token::type::kw_extension:
    if (parsed_attributes)
      __builtin_trap();  // TODO(compnerd) diagnose invalid declaration
    return parse_extension_declaration();
  case token::type::kw_subscript:
    if (parsed_declaration_modifiers)
      __builtin_trap();  // TODO(compnerd) diagnose invalid declaration
    return parse_subscript_declaration();
  case token::type::kw_prefix:
  case token::type::kw_postfix:
  case token::type::kw_infix:
    return parse_operator_declaration();
  }
}

// import-declaration → attributes[opt] 'import' import-kind[opt] import-path
//
// import-kind → 'typealias'
// import-kind → 'struct'
// import-kind → 'class'
// import-kind → 'enum'
// import-kind → 'protocol'
// import-kind → 'var'
// import-kind → 'func'
//
// import-path → import-path-identifier
// import-path → import-path-identifier '.' import-path
//
// import-path-identifier → identifier
// import-path-identifier → operator
parse::result<ast::declaration> parser::parse_import_declaration() {
  assert(lexer_.head().is<token::type::kw_import>() && "expected 'import'");

  parse::result<ast::declaration> import_declaration;

  location begin = lexer_.next().location().start();

  semantic::import_kind kind;
  switch (lexer_.head()) {
  default:
    kind = semantic::ik_module;
    break;
  case token::type::kw_typealias:
    kind = semantic::ik_typealias;
    break;
  case token::type::kw_struct:
    kind = semantic::ik_struct;
    break;
  case token::type::kw_class:
    kind = semantic::ik_class;
    break;
  case token::type::kw_enum:
    kind = semantic::ik_enum;
    break;
  case token::type::kw_protocol:
    kind = semantic::ik_protocol;
    break;
  case token::type::kw_var:
    kind = semantic::ik_var;
    break;
  case token::type::kw_func:
    kind = semantic::ik_func;
    break;
  }

  if (not (kind == semantic::ik_module))
    lexer_.next();

  const char32_t *buffer = lexer_.head().value().data();
  location start = lexer_.head().location().start();
  location end = lexer_.head().location().start();
  while (not lexer_.head().is<token::type::eof>()) {
    if (not lexer_.head().is<token::type::op>() and
        not lexer_.head().is<token::type::identifier>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_identifier);
      return import_declaration;
    }

    end = lexer_.next().location().end();

    if (not lexer_.head().is<token::type::period>())
      break;
    lexer_.next();
  }

  if (start == end) {
    diagnose(begin, diagnostic::err_expected_identifier);
    return import_declaration;
  }

  size_t length = end - start;
  std::u32string_view import_path(buffer, length);

  import_declaration =
      semantic_analyzer_.import_declaration(range(begin, end), import_path);
  return import_declaration;
}

// constant-declaration → attributes[opt] declaration-modifiers[opt] 'let' pattern-initializer-list
parse::result<ast::declaration> parser::parse_constant_declaration() {
  assert(lexer_.head().is<token::type::kw_let>() && "expected 'let'");
  lexer_.next();

  std::vector<ast::declaration *> initializer_list;
  if (not parse_pattern_initializer_list(initializer_list)) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_pattern);
    return parse::result<ast::declaration>();
  }

  // TODO(compnerd) handle multiple declarations
  assert(initializer_list.size() <= 1 && "cannot construct a brace statement");
  return parse::result<ast::declaration>(*initializer_list.begin());
}

// variable-declaration → variable-declaration-head pattern-initializer-list
// variable-declaration → variable-declaration-head variable-name type-annotation code-block
// variable-declaration → variable-declaration-head variable-name type-annotation getter-setter-block
// variable-declaration → variable-declaration-head variable-name type-annotation getter-setter-keyword-block
// variable-declaration → variable-declaration-head variable-name type-annotation initializer[opt] willSet-didSet-block
//
// variable-declaration-head → attributes[opt] declaration-modifiers[opt] 'var'
//
// variable-name → identifier
//
// getter-setter-block → '{' getter-clause setter-clause[opt] '}'
// getter-setter-block → '{' setter-clause getter-clause '}'
// getter-clause → attributes[opt] 'get' code-block
// setter-clause → attributes[opt] 'set' setter-name[opt] code-block
// setter-name → '(' identifier ')'
//
// getter-setter-keyword-block → '{' getter-keyword-clause setter-keyword-clause[opt] '}'
// getter-setter-keyword-block → '{' setter-keyword-clause getter-keyword-clause '}'
// getter-keyword-clause → attributes[opt] 'get'
// setter-keyword-clause → attributes[opt] 'set'
//
// willSet-didSet-block → '{' willSet-clause didSet-clause[opt] '}'
// willSet-didSet-block → '{' didSet-clause willSet-clause '}'
// willSet-clause → attributes[opt] 'willSet' setter-name[opt] code-block
// didSet-clause → attributes[opt] 'didSet' setter-name[opt] code-block
parse::result<ast::declaration> parser::parse_variable_declaration() {
  assert(lexer_.head().is<token::type::kw_var>() && "expected 'var'");

  lexer_.next();

  parse::result<ast::declaration> variable_declaration;
  parse::result<ast::pattern> pattern;

  location pattern_location = lexer_.head().location().start();
  if (not (pattern = parse_pattern())) {
    diagnose(pattern_location, diagnostic::err_expected_pattern);
    return variable_declaration;
  }

  if (lexer_.head().is<token::type::colon>()) {
    location type_location = lexer_.head().location().start();
    if (not parse_type_annotation()) {
      diagnose(type_location, diagnostic::err_expected_type);
      return variable_declaration;
    }
  } else if (not (ast::isa<ast::pattern::type::typed>(pattern) or
                  ast::isa<ast::pattern::type::tuple>(pattern)) and
             not lexer_.head().is<token::type::equal>()) {
    // TODO(compnerd) only issue this diagnostic if the pattern is not a typed
    diagnose(lexer_.head().location().start(),
             diagnostic::err_type_annotation_missing_in)
        << "pattern";
    return variable_declaration;
  }

  parse::result<ast::expression> initializer;
  if (lexer_.head().is<token::type::equal>()) {
    token equal = lexer_.head();

    if (not (initializer = parse_initializer())) {
      diagnose(equal.location().end(),
               diagnostic::err_expected_initial_value_after)
          << equal;
      return variable_declaration;
    }
  }

  // TODO(compnerd) ensure that the pattern was typed

  variable_declaration =
      semantic_analyzer_.variable_declaration(pattern, initializer);
  return variable_declaration;
}

// typealias-declaration → typealias-head typealias-assignment
// typealias-head → attributes[opt] access-level-modifier[opt] 'typealias' typealias-name
// typealias-name → identifier
// typealias-assignment → '=' type
parse::result<ast::declaration> parser::parse_typealias_declaration() {
  assert(lexer_.head().is<token::type::kw_typealias>() && "expected 'typealias'");

  parse::result<ast::declaration> typealias;

  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_in)
        << "typealias declaration";
    return typealias;
  }
  token name = lexer_.next();

  if (not lexer_.head().is<token::type::equal>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::equal, U"=", location(), location())
        << "typealias declaration";
    return typealias;
  }
  lexer_.next();

  location position = lexer_.head().location().start();
  if (not parse_type()) {
    diagnose(position, diagnostic::err_expected_type_in)
        << "typealias declaration";
    return typealias;
  }

  typealias = semantic_analyzer_.typealias_declaration(name.value(), U"");
  return typealias;
}

// function-declaration → function-head function-name generic-parameter-clause[opt] function-signature function-body
//
// function-head → attributes[opt] declaration-modifiers[opt] 'func'
//
// function-name → identifier
// function-name → operator
//
// function-signature → parameter-clauses function-result[opt]
//
// function-body → code-block
parse::result<ast::declaration> parser::parse_function_declaration() {
  assert(lexer_.head().is<token::type::kw_func>() && "expected 'func'");

  parse::result<ast::declaration> function;
  parse::result<ast::type> result_type;
  std::vector<ast::pattern *> parameter_clauses;

  // TODO(compnerd) track this location
  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>() and
      not lexer_.head().is<token::type::op>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_in)
        << "function declaration";
    return function;
  }

  swift::token name = lexer_.next();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::function);

  if (less(lexer_.head()))
    parse_generic_parameter_clause();

  if (not lexer_.head().is<token::type::l_paren>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::l_paren, U"(", location(), location())
        << "argument list of function declaration";
    return function;
  }

  parameter_clauses = parse_parameter_clauses();
  if (parameter_clauses.empty()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_argument_list);
    return function;
  }

  if (name == token::type::op) {
    const ast::pattern_tuple &clause_tuple =
        *static_cast<const ast::pattern_tuple *>(parameter_clauses.front());
    if (clause_tuple.elements().size() == 0) {
      diagnose(name.location(),
               diagnostic::err_operators_must_have_one_or_two_arguments);
      return function;
    }
  }

  if (lexer_.head().is<token::type::arrow>())
    result_type = parse_function_result();

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::r_brace, U"{", location(), location())
        << "body of function declaration";
    return function;
  }

  // NOTE(compnerd) if the parsing of the body failed, we have already emitted a
  // diagnostic.
  // TODO(compnerd) ensure that a diagnostic was presented, if not, emit a
  // secondary one.
  if (parse::result<ast::statement> body = parse_code_block())
    function =
        semantic_analyzer_.function_declaration(name.value(), parameter_clauses,
                                                result_type, body);
  return function;
}

// parameter-clauses → parameter-clause parameter-clauses[opt]
std::vector<ast::pattern *> parser::parse_parameter_clauses() {
  std::vector<ast::pattern *> clauses;

  do
    if (auto clause = parse_parameter_clause())
      clauses.push_back(*clause);
    else
      break;
  while (lexer_.head().is<token::type::l_paren>());

  return clauses;
}

// parameter-clause → '(' ')'
// parameter-clause → '(' parameter-list '...'[opt] ')'
parse::result<ast::pattern> parser::parse_parameter_clause() {
  assert(lexer_.head().is<token::type::l_paren>() && "expected '('");

  parse::result<ast::pattern> parameter_clause;

  lexer_.next();

  std::vector<ast::pattern *> parameters = parse_parameter_list();
  if (parameters.empty() and ellipsis(lexer_.head())) {
    diagnose(lexer_.head().location(),
             diagnostic::err_expected_parameter_type_following)
        << token(token::type::colon, U":", location(), location());
    return parameter_clause;
  }
  if (ellipsis(lexer_.head()) and
      not lexer_.peek().is<token::type::r_paren>()) {
    diagnose(lexer_.head().location(),
             diagnostic::err_token_must_be_on_the_last_parameter)
        << lexer_.head();
    return parameter_clause;
  }

  if (not lexer_.head().is<token::type::r_paren>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token)
        << token(token::type::r_paren, U")", location(), location());
    return parameter_clause;
  }

  lexer_.next();

  parameter_clause = semantic_analyzer_.pattern_tuple(parameters);
  return parameter_clause;
}

// parameter-list → parameter
// parameter-list → parameter ',' parameter-list
std::vector<ast::pattern *> parser::parse_parameter_list() {
  std::vector<ast::pattern *> parameter_list;

  while (true) {
    if (auto parameter = parse_parameter()) {
      parameter_list.push_back(*parameter);
      if (not lexer_.head().is<token::type::comma>())
        return parameter_list;
      lexer_.next();
      continue;
    }

    consume_until(token::type::r_paren);
    return std::vector<ast::pattern *>();
  }
}

// parameter → 'inout'[opt] 'let'[opt] '#'[opt] external-parameter-name[opt] local-parameter-name type-annotation default-argument-clause[opt]
// parameter → 'inout'[opt] 'var' '#'[opt] external-parameter-name[opt] local-parameter-name type-annotation default-argument-clause[opt]
// parameter → attributes[opt] type
//
// external-parameter-name → identifier
// external-parameter-name → '_'
//
// local-parameter-name → identifier
// local-parameter-name → '_'
//
// default-argument-clause → '=' expression
parse::result<ast::pattern> parser::parse_parameter() {
  parse::result<ast::pattern> parameter;
  token external_name;
  parse::result<ast::pattern> external_parameter_name;
  token local_name;
  parse::result<ast::pattern> local_parameter_name;
  parse::result<ast::type> type;

  // attributes[opt] type default-argument-clause[opt]
  if (lexer_.head().is<token::type::at>() or
      (lexer_.head().is<token::type::identifier>() and
       (lexer_.peek().is<token::type::equal>() or
        lexer_.peek().is<token::type::comma>() or
        lexer_.peek().is<token::type::r_paren>()))) {
    if (lexer_.head().is<token::type::at>())
      parse_attributes(/*is_declaration=*/false);

    if (auto type = parse_type()) {
      local_parameter_name =
          semantic_analyzer_.pattern_named(std::u32string_view(),
                                           /*implicit=*/true);
      parameter =
          semantic_analyzer_.pattern_typed(local_parameter_name, type);
    }

    if (lexer_.head().is<token::type::equal>())
      __builtin_trap();

    return parameter;
  }

  std::bitset<3> variable_specifiers;
  enum { ps_none, ps_inout, ps_let, ps_var };

  switch (lexer_.head()) {
  default:
    variable_specifiers = ps_let;
    break;
  case token::type::kw_inout:
    variable_specifiers.set(ps_inout);
    lexer_.next();

    if (lexer_.head().is<token::type::kw_let>() or
        lexer_.head().is<token::type::kw_var>()) {
      diagnose(lexer_.head().location(),
               diagnostic::err_parameter_may_not_have_multiple_specifiers);
      return parameter;
    }

    break;
  case token::type::kw_let:
    variable_specifiers.set(ps_let);
    lexer_.next();

    if (lexer_.head().is<token::type::kw_inout>() or
        lexer_.head().is<token::type::kw_var>()) {
      diagnose(lexer_.head().location(),
               diagnostic::err_parameter_may_not_have_multiple_specifiers);
      return parameter;
    }
    break;
  case token::type::kw_var:
    variable_specifiers.set(ps_var);
    lexer_.next();

    if (lexer_.head().is<token::type::kw_inout>() or
        lexer_.head().is<token::type::kw_let>()) {
      diagnose(lexer_.head().location(),
               diagnostic::err_parameter_may_not_have_multiple_specifiers);
      return parameter;
    }
    break;
  }

  if (lexer_.head().is<token::type::hash>())
    __builtin_trap();

  // external-parameter-name[opt] local-parameter-name
  if (lexer_.head().is<token::type::identifier>() and
      lexer_.peek().is<token::type::identifier>()) {
    external_name = lexer_.next();
    external_parameter_name =
        semantic_analyzer_.pattern_named(external_name.value(),
                                         /*implicit=*/false);
  }

  if (lexer_.head().is<token::type::identifier>()) {
    local_name = lexer_.next();
    local_parameter_name = semantic_analyzer_.pattern_named(local_name.value(),
                                                            /*implicit=*/false);

    if (local_name.value() == external_name.value())
      diagnose(external_name.location(),
               diagnostic::warn_parameter_name_can_be_expression_more_succinctly_as)
          << external_name;

    if (not lexer_.head().is<token::type::colon>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_type_annotation_missing_in)
          << "pattern";
      return parameter;
    }

    if (not (type = parse_type_annotation()))
      return parameter;
    if (variable_specifiers.test(ps_inout))
      type = semantic_analyzer_.type_inout(*type);

    parameter = semantic_analyzer_.pattern_typed(local_parameter_name, *type);

    if (lexer_.head().is<token::type::equal>())
      __builtin_trap();
  }

  return parameter;
}

// function-result → '->' attributes[opt] type
parse::result<ast::type> parser::parse_function_result() {
  assert(lexer_.head().is<token::type::arrow>() && "expected '->'");

  lexer_.next();

  parse_attributes(/*is_declaration=*/false);

  location type_location = lexer_.head().location().start();
  if (auto type = parse_type())
    return type;

  diagnose(type_location, diagnostic::err_expected_type_for_function_result);
  return parse::result<ast::type>();
}

// code-block → '{' statements[opt] '}'
parse::result<ast::statement> parser::parse_code_block() {
  assert(lexer_.head().is<token::type::l_brace>() && "expected '{'");

  parse::result<ast::statement> code_block;

  // TODO(compnerd) track this location
  lexer_.next();

  code_block = parse_statements();

  if (not lexer_.head().is<token::type::r_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_at)
        << token(token::type::r_brace, U"}", location(), location())
        << "end of brace statement";
    return code_block;
  }
  lexer_.next();

  return code_block;
}

// enum-declaration → attributes[opt] access-level-modifier[opt] union-style-enum
// enum-declaration → attributes[opt] access-level-modifier[opt] raw-value-style-enum
//
// union-style-enum → 'enum' enum-name generic-parameter-clause[opt] type-inheritance-clause[opt] '{' union-style-enum-members[opt] '}'
//
// union-style-enum-members → union-style-enum-member union-style-enum-members[opt]
//
// union-style-enum-member → declaration
// union-style-enum-member → union-style-enum-case-clause
//
// union-style-enum-case-clause → attributes[opt] 'case' union-style-enum-case-list
//
// union-style-enum-case-list → union-style-enum-case
// union-style-enum-case-list → union-style-enum-case ',' union-style-enum-case-list
//
// union-style-enum-case → enum-case-name tuple-type[opt]
//
// enum-name → identifier
//
// enum-case-name → identifier
//
// raw-value-style-enum → 'enum' enum-name generic-parameter-clause[opt] type-inheritance-clause '{' raw-value-style-enum-members '}'
//
// raw-value-style-enum-members → raw-value-style-enum-member raw-value-style-enum-members[opt]
//
// raw-value-style-enum-member → declaration
// raw-value-style-enum-member → raw-value-style-enum-case-clause
//
// raw-value-style-enum-case-clause → attributes[opt] 'case' raw-value-style-enum-case-list
//
// raw-value-style-enum-case-list → raw-value-style-enum-case ',' raw-value-style-enum-case-list
// raw-value-style-enum-case-list → raw-value-style-enum-case
//
// raw-value-style-enum-case → enum-case-name raw-value-assignment[opt]
//
// raw-value-assignment → '=' literal
parse::result<ast::declaration> parser::parse_enum_declaration() {
#if 0
  assert(lexer_.head().is<token::type::kw_enum>() && "expected 'enum'");

  parse::result<ast::declaration> enum_declaration;
  std::vector<ast::declaration *> members;

  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_in)
        << "enum declaration";
    return enum_declaration;
  }
  token enum_name = lexer_.next();

  if (less(lexer_.head()))
    parse_generic_parameter_clause();

  parse_type_inheritance_clause();

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(),
                               diagnostic::err_expected_token_in)
        << token(token::type::l_brace, U"{", location(), location()) << "enum";
    return enum_declaration;
  }
  lexer_.next();

  for (bool parsed = false; not lexer_.head().is<token::type::r_brace>();
       parsed = true) {
    if (lexer_.head().is<token::type::at>() or
        lexer_.head().is<token::type::kw_case>()) {
      parse_attributes(/*is_declaration=*/true);

      if (not lexer_.head().is<token::type::kw_case>())
        __builtin_trap();  // TODO(compnerd) diagnose missing 'case'
      lexer_.next();

      while (not lexer_.head().is<token::type::eof>()) {
        if (not lexer_.head().is<token::type::identifier>()) {
          diagnose(lexer_.head().location().start(),
                                     diagnostic::err_expected_identifier_in)
              << "enum case declaration";
          return enum_declaration;
        }
        token case_name = lexer_.next();

        if (lexer_.head().is<token::type::l_paren>()) {
          location type_location = lexer_.peek().location().start();
          if (not parse_tuple_type()) {
            diagnose(type_location,
                                       diagnostic::err_expected_type);
            return enum_declaration;
          }
        } else if (lexer_.head().is<token::type::equal>()) {
          lexer_.next();

          if (not lexer_.head().is<token::type::literal>()) {
            diagnose(lexer_.head().location().start(),
                     diagnostic::err_raw_value_for_enum_case_must_be_a_literal);
            return enum_declaration;
          }
          lexer_.next();
        }

        ast::declaration *enumeration_element =
            semantic_analyzer_.enumeration_element_declaration(
                case_name.value());
        if (not enumeration_element)
          return enum_declaration;
        members.push_back(enumeration_element);

        if (not lexer_.head().is<token::type::comma>())
          break;
        lexer_.next();
      }
    } else {
      if (auto declaration = parse_declaration())
        members.push_back(*declaration);
      else if (parsed)
        __builtin_trap();  // TODO(compnerd) diagnose bad *-style-enum-member
    }
  }

  if (not lexer_.head().is<token::type::r_brace>())
    __builtin_trap();  // TODO(compnerd) diagnose missing '}'
  lexer_.next();

  enum_declaration =
      semantic_analyzer_.enum_declaration(enum_name.value(), members);
  return enum_declaration;
#endif
  __builtin_trap();
}

// enum-case-name → identifier
parse::result<ast::declaration> parser::parse_enum_case_name() {
  __builtin_trap();
}

// struct-declaration → attributes[opt] access-level-modifier[opt] 'struct' struct-name generic-parameter-clause[opt] type-inheritance-clause[opt] struct-body
//
// struct-name → identifier
//
// struct-body → '{' declarations[opt] '}'
parse::result<ast::declaration> parser::parse_struct_declaration() {
  assert(lexer_.head().is<token::type::kw_struct>() && "expected 'struct'");

  parse::result<ast::declaration> struct_declaration;

  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_in)
        << "struct declaration";
    return struct_declaration;
  }
  token struct_name = lexer_.next();

  if (less(lexer_.head()))
    parse_generic_parameter_clause();

  parse_type_inheritance_clause();

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::l_brace, U"{", location(), location())
        << "struct";
    return struct_declaration;
  }
  lexer_.next();

  parse::result<ast::statement> declarations = parse_declarations();

  if (not lexer_.head().is<token::type::r_brace>())
    __builtin_trap();  // TODO(compnerd) diagnose missing '}'
  lexer_.next();

  struct_declaration =
      semantic_analyzer_.struct_declaration(struct_name.value(), *declarations);
  return struct_declaration;
}

// class-declaration → attributes[opt] access-level-modifier[opt] 'class' class-name generic-parameter-clause[opt] type-inheritance-clause[opt] class-body
// class-name → identifier
// class-body → '{' declarations[opt] '}'
parse::result<ast::declaration> parser::parse_class_declaration() {
  assert(lexer_.head().is<token::type::kw_class>() && "expected 'class'");

  parse::result<ast::declaration> class_declaration;

  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_in)
        << "class declaration";
    return class_declaration;
  }
  token class_name = lexer_.next();

  if (less(lexer_.head()))
    parse_generic_parameter_clause();

  parse_type_inheritance_clause();

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::l_brace, U"{", location(), location()) << "class";
    return class_declaration;
  }
  lexer_.next();

  parse::result<ast::statement> declarations = parse_declarations();

  if (not lexer_.head().is<token::type::r_brace>())
    __builtin_trap();  // TODO(compnerd) diagnose invalid class-body
  lexer_.next();

  class_declaration =
      semantic_analyzer_.class_declaration(class_name.value(), *declarations);
  return class_declaration;
}

// protocol-declaration → attributes[opt] access-level-modifier[opt] 'protocol' protocol-name type-inheritance-clause[opt] protocol-body
//
// protocol-name → identifier
// protocol-body → '{' protocol-member-declarations[opt] '}'
parse::result<ast::declaration> parser::parse_protocol_declaration() {
  assert(lexer_.head().is<token::type::kw_protocol>() && "expected 'protocol'");

  parse::result<ast::declaration> protocol_declaration;

  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_identifier_in)
        << "protocol declaration";
    return protocol_declaration;
  }
  token protocol_name = lexer_.next();

  parse_type_inheritance_clause();

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::l_brace, U"{", location(), location())
        << "protocol type";
    return protocol_declaration;
  }
  lexer_.next();

  parse_protocol_member_declarations();

  if (not lexer_.head().is<token::type::r_brace>())
    __builtin_trap();  // TODO(compnerd) diagnose invalid protocol-declaration
  lexer_.next();

  protocol_declaration =
      semantic_analyzer_.protocol_declaration(protocol_name.value(), nullptr);
  return protocol_declaration;
}

// protocol-member-declarations → protocol-member-declaration protocol-member-declarations[opt]
bool parser::parse_protocol_member_declarations() {
  for (bool parsed = false; ; parsed = true)
    if (not parse_protocol_member_declaration())
      return parsed;
}

// protocol-member-declaration → protocol-property-declaration
// protocol-member-declaration → protocol-method-declaration
// protocol-member-declaration → protocol-initializer-declaration
// protocol-member-declaration → protocol-subscript-declaration
// protocol-member-declaration → protocol-associated-type-declaration
//
// protocol-property-declaration → variable-declaration-head variable-name type-annotation getter-setter-keyword-block
//
// protocol-method-declaration → function-head function-name generic-parameter-clause[opt] function-signature
//
// protocol-initializer-declaration → initializer-head generic-parameter-clause[opt] parameter-clause
//
// protocol-subscript-declaration → subscript-head subscript-result getter-setter-keyword-block
//
// protocol-associated-type-declaration → typealias-head type-inheritance-clause[opt] typealias-assignment[opt]
bool parser::parse_protocol_member_declaration() {
  bool has_attributes = parse_attributes(/*is_declaration=*/true);
  bool has_modifiers = parse_declaration_modifiers();

  switch (lexer_.head()) {
  default:
    if (has_attributes or has_modifiers)
      __builtin_trap();  // TODO(compnerd) diagnose invalid
                         // protocol-member-declaration
    return false;
  case token::type::kw_var:
    __builtin_trap();
  case token::type::kw_func:
    __builtin_trap();
  case token::type::kw_init:
    __builtin_trap();
  case token::type::kw_subscript:
    __builtin_trap();
  }

  return true;
}

// type-inheritance-clause → ':' class-requirement ',' type-inheritance-list
// type-inheritance-clause → ':' class-requirement
// type-inheritance-clause → ':' type-inheritance-list
//
// type-inheritance-list → type-identifier
// type-inheritance-list → type-identifier ',' type-inheritance-list
//
// class-requirement → 'class'
bool parser::parse_type_inheritance_clause() {
  if (not lexer_.head().is<token::type::colon>())
    return false;
  lexer_.next();

  if (lexer_.head().is<token::type::kw_class>()) {
    lexer_.next();
    if (not lexer_.head().is<token::type::comma>())
      return true;
    lexer_.next();
  }

  while (true) {
    if (not parse_type_identifier())
      __builtin_trap();  // TODO(compnerd) diagnose incorrect
                         // type-inheritance-clause
    if (not lexer_.head().is<token::type::comma>())
      break;
    lexer_.next();
  }

  return true;
}

// initializer-declaration → initializer-head generic-parameter-clause[opt] parameter-clause initializer-body
//
// initializer-head → attributes[opt] declaration-modifiers[opt] 'init'
//
// initializer-body → code-block
parse::result<ast::declaration> parser::parse_initializer_declaration() {
  __builtin_trap();
}

// deinitializer-declaration → attributes[opt] 'deinit' code-block
parse::result<ast::declaration> parser::parse_deinitializer_declaration() {
  __builtin_trap();
}

// extension-declaration → access-level-modifier[opt] 'extension' type-identifier type-inheritance-clause[opt] extension-body
//
// extension-body → '{' declarations[opt] '}'
parse::result<ast::declaration> parser::parse_extension_declaration() {
  __builtin_trap();
}

// subscript-declaration → subscript-head subscript-result code-block
// subscript-declaration → subscript-head subscript-result getter-setter-block
// subscript-declaration → subscript-head subscript-result getter-setter-keyword-block
//
// subscript-head → attributes[opt] declaration-modifiers[opt] 'subscript' parameter-clause
//
// subscript-result → attributes[opt] type
parse::result<ast::declaration> parser::parse_subscript_declaration() {
  __builtin_trap();
}

// operator-declaration → prefix-operator-declaration
// operator-declaration → postfix-operator-declaration
// operator-declaration → infix-operator-declaration
//
// prefix-operator-declaration → 'prefix' 'operator' operator '{' '}'
//
// postfix-operator-declaration → 'postfix' 'operator' operator '{' '}'
//
// infix-operator-declaration → 'infix' 'operator' operator '{' infix-operator-attributes[opt] '}'
//
// infix-operator-attributes → 'precedence' precedence-level
//
// precedence-level → a decimal integer between 0 and 255, inclusive
//
// associativity-clause → 'associativity' associativity
//
// associativity → 'left'
// associativity → 'right'
// associativity → 'none'
parse::result<ast::declaration> parser::parse_operator_declaration() {
  assert((lexer_.head().is<token::type::kw_infix>() or
          lexer_.head().is<token::type::kw_prefix>() or
          lexer_.head().is<token::type::kw_postfix>()) &&
         "expected 'infix', 'prefix', or 'postfix'");

  parse::result<ast::declaration> operator_declaration;

  token operator_type = lexer_.next();

  if (not lexer_.head().is<token::type::kw_operator>()) {
    diagnose(lexer_.head().location(),
             diagnostic::err_token_requires_a_function_with_an_operator_identifier)
        << operator_type;
    return operator_declaration;
  }
  lexer_.next();

  if (not lexer_.head().is<token::type::identifier>() and
      not lexer_.head().is<token::type::exclaim>() and
      not lexer_.head().is<token::type::amp>() and
      not lexer_.head().is<token::type::op>()) {
    diagnose(lexer_.head().location(),
             diagnostic::err_expected_operator_name_in_operator_declaration);
    return operator_declaration;
  }
  lexer_.next();

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location(),
             diagnostic::err_expected_token_after_syntax)
        << token(token::type::l_brace, U"{", location(), location())
        << "operator name in 'operator' declaration";
    return operator_declaration;
  }
  lexer_.next();

  __builtin_trap();
}

// loop-statement → for-statement
// loop-statement → for-in-statement
// loop-statement → while-statement
// loop-statement → do-while-statement
parse::result<ast::statement> parser::parse_loop_statement() {
  assert((lexer_.head().is<token::type::kw_for>() or
          lexer_.head().is<token::type::kw_while>() or
          lexer_.head().is<token::type::kw_repeat>()) &&
         "expected 'for', 'while', or 'repeat'");

  switch (lexer_.head()) {
  default: swift_unreachable("invalid loop-statement");
  case token::type::kw_for:
    if (lexer_.peek().is<token::type::l_paren>() or
        lexer_.peek().is<token::type::colon>())
      return parse_for_statement();
    return parse_for_in_statement();
  case token::type::kw_while:
    return parse_while_statement();
  case token::type::kw_repeat:
    return parse_repeat_while_statement();
  }
}

// for-statement → 'for' for-init[opt] ';' expression[opt] ';' expression[opt] code-block
// for-statement → 'for' '(' for-init[opt] ';' expression[opt] ';'expression[opt] ')' code-block
//
// for-init → variable-declaration
// for-init → expression-list
parse::result<ast::statement> parser::parse_for_statement() {
  __builtin_trap();
}

// for-in-statement → 'for' pattern 'in' expression code-block
parse::result<ast::statement> parser::parse_for_in_statement() {
  assert(lexer_.head().is<token::type::kw_for>() && "expected 'for'");

  parse::result<ast::statement> for_each_statement;
  parse::result<ast::expression> expression;

  lexer_.next();

  parse::result<ast::pattern> pattern = parse_pattern();
  if (not pattern) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_initialization_in)
        << "a for-each statement";
    return for_each_statement;
  }

  if (not lexer_.head().is<token::type::kw_in>())
    __builtin_trap();  // TODO(compnerd) diagnose invalid for-in-statement
  lexer_.next();

  if (not (expression = parse_expression())) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_typed_expression_for)
        << "SequenceType"
        << "for-each loop";
    return for_each_statement;
  }

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_type)
        << token(token::type::l_brace, U"{", location(), location())
        << "to start the body of for-each loop";
    return for_each_statement;
  }

  if (parse::result<ast::statement> body = parse_code_block())
    for_each_statement =
        semantic_analyzer_.for_in_statement(pattern, expression, body);
  return for_each_statement;
}

// while-statement → 'while' while-condition code-block
parse::result<ast::statement> parser::parse_while_statement() {
  assert(lexer_.head().is<token::type::kw_while>() && "expected 'while'");

  parse::result<ast::statement> while_statement;

  lexer_.next();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::statement);

  parse::result<ast::statement> condition = parse_while_condition();
  if (not condition) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_expression_or_binding_in)
        << "'while' condition";
    return while_statement;
  }

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_after_syntax)
        << token(token::type::l_brace, U"{", location(), location())
        << "'while' condition";
    return while_statement;
  }

  if (parse::result<ast::statement> body = parse_code_block())
    while_statement = semantic_analyzer_.while_statement(condition, body);
  return while_statement;
}

// repeat-while-statement → 'repat' code-block 'while' expression
parse::result<ast::statement> parser::parse_repeat_while_statement() {
  assert(lexer_.head().is<token::type::kw_repeat>() && "expected 'repeat'");

  parse::result<ast::statement> repeat_while;

  token repeat = lexer_.next();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::statement);

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_after_token)
        << token(token::type::l_brace, U"{", location(), location()) << repeat;
    return repeat_while;
  }

  // NOTE(compnerd) if the parsing of the body failed, we have already emitted a
  // diagnostic.
  // TODO(compnerd) ensure that a diagnostic was presented, if not, emit a
  // secondary one.
  parse::result<ast::statement> body = parse_code_block();
  if (not body)
    return repeat_while;

  if (not lexer_.head().is<token::type::kw_while>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_in)
        << token(token::type::kw_while, U"while", location(), location())
        << "'repeat-while' loop";
    return repeat_while;
  }
  lexer_.next();

  parse::result<ast::expression> condition = parse_expression();
  if (not condition) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_expression_in)
        << "'repeat-while' condition";
    return repeat_while;
  }

  repeat_while = semantic_analyzer_.repeat_while_statement(body, condition);
  return repeat_while;
}

// while-condition → expression
// while-condition → declaration
parse::result<ast::statement> parser::parse_while_condition() {
  parse::result<ast::statement> while_condition;

  if (auto expression = parse_expression())
    return (while_condition = expression);

  if (auto declaration = parse_declaration())
    return (while_condition = declaration);

  return while_condition;
}

// branch-statement → if-statement
// branch-statement → switch-statement
parse::result<ast::statement> parser::parse_branch_statement() {
  assert((lexer_.head().is<token::type::kw_if>() or
          lexer_.head().is<token::type::kw_switch>()) &&
         "expected 'if' or 'switch'");

  switch (lexer_.head()) {
  default: swift_unreachable("invalid branch statement");
  case token::type::kw_if:
    return parse_if_statement();
  case token::type::kw_switch:
    return parse_switch_statement();
  }
}

// if-statement → 'if' if-condition code-block else-clause[opt]
//
// if-condition → expression
// if-condition → declaration
//
// else-clause → 'else' code-block
// else-clause → 'else' if-statement
parse::result<ast::statement> parser::parse_if_statement() {
  assert(lexer_.head().is<token::type::kw_if>() && "expected 'if'");

  parse::result<ast::statement> if_statement;

  lexer_.next();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::branch);

  location condition_location = lexer_.head().location().start();

  parse::result<ast::expression> expression;
  parse::result<ast::declaration> declaration;
  if ((expression = parse_expression())) {
  } else if ((declaration = parse_declaration())) {
  } else {
    diagnose(condition_location,
             diagnostic::err_expected_expression_or_binding_in)
        << "'if' condition";
    return if_statement;
  }

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_after_syntax)
        << token(token::type::l_brace, U"{", location(), location())
        << "'if' condition";
    return if_statement;
  }

  parse::result<ast::statement> true_clause = parse_code_block();
  if (not true_clause)
    return if_statement;

  parse::result<ast::statement> false_clause;
  if (lexer_.head().is<token::type::kw_else>()) {
    token else_tok = lexer_.next();

    switch (lexer_.head()) {
    default:
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_token_after_token)
          << token(token::type::l_brace, U"{", location(), location())
          << else_tok;
      return if_statement;

    case token::type::kw_if: {
      parse::result<ast::statement> statement = parse_if_statement();
      if (not statement)
        return if_statement;

      // TODO(compnerd) permit IfStatement to take a brace statement or a
      // statement.
      std::vector<ast::statement *> clause(1, *statement);
      false_clause = semantic_analyzer_.statements(clause);
      break;
    }
    case token::type::l_brace:
      false_clause = parse_code_block();
      break;
    }
  }

  assert((expression or declaration) && "expected expression or declaration");
  // TODO(compnerd) track parenthesis locations
  // TODO(compnerd) track brace locations
  if_statement =
      semantic_analyzer_
          .if_statement(expression
                            ? static_cast<ast::statement *>(expression)
                            : static_cast<ast::statement *>(*declaration),
                        true_clause, false_clause);
  return if_statement;
}

// switch-statement → 'switch' expression '{' switch-cases[opt] '}'
//
// switch-cases → switch-case switch-cases[opt]
// switch-case → case-label statements
// switch-case → default-label statements
// switch-case → case-label ';'
// switch-case → default-label ';'
//
// case-label → 'case' case-item-list ':'
// case-item-list → pattern guard-clause[opt]
// case-item-list → pattern guard-clause[opt] ',' case-item-list
//
// default-label → 'default' ':'
//
// guard-clause → 'where' guard-expression
//
// guard-expression → expression
parse::result<ast::statement> parser::parse_switch_statement() {
  assert(lexer_.head().is<token::type::kw_switch>() && "expected 'switch'");

  parse::result<ast::statement> switch_statement;
  parse::result<ast::expression> control_expression;

  lexer_.next();

  semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                             semantic::scope::type::switch_statement);

  if (not (control_expression = parse_expression())) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_expression_in)
        << "'switch' statement";
    return switch_statement;
  }

  if (not lexer_.head().is<token::type::l_brace>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_after_syntax)
        << token(token::type::l_brace, U"{", location(), location())
        << "'switch' subject expression";
    return switch_statement;
  }
  lexer_.next();

  std::vector<ast::switch_statement::case_item> cases;

  // TODO(compnerd) parse optional switch_cases
  for (bool seen_default = false;
       not lexer_.head().is<token::type::r_brace>();) {
    std::vector<std::tuple<ast::pattern *, ast::expression *>> case_item_list;

    token case_token = lexer_.next();
    if (seen_default and case_token.is<token::type::kw_case>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_additional_case_blocks_cannot_appear_after_default);
      return switch_statement;
    }

    if (case_token.is<token::type::kw_default>()) {
      seen_default = true;
      case_item_list.emplace_back(semantic_analyzer_.pattern_any(), nullptr);
    } else {
      while (not lexer_.head().is<token::type::colon>()) {
        location pattern_location = lexer_.head().location().start();
        parse::result<ast::pattern> pattern;
        parse::result<ast::expression> guard;

        semantic::scope_raii scope(semantic_analyzer_.current_scope(),
                                   semantic::scope::type::switch_case);

        pattern = parse_pattern();
        if (not pattern) {
          diagnose(pattern_location, diagnostic::err_expected_pattern);
          return switch_statement;
        }

        if (lexer_.head().is<token::type::kw_where>()) {
          lexer_.next();

          location guard_clause_location = lexer_.head().location().start();
          guard = parse_expression();
          if (not guard) {
            diagnose(guard_clause_location,
                     diagnostic::err_expected_expression_for_syntax)
                << "'where' guard of 'case'";
            return switch_statement;
          }
        }

        case_item_list.emplace_back(pattern, guard);

        if (not lexer_.head().is<token::type::comma>())
          break;
        lexer_.next();
      }
    }

    if (not lexer_.head().is<token::type::colon>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_token_after_syntax)
          << token(token::type::colon, U":", location(), location())
          << case_token;
      return switch_statement;
    }
    token colon_token = lexer_.next();

    parse::result<ast::statement> statements = parse_statements();
    if (not statements) {
      range case_range(case_token.location().start(),
                       colon_token.location().end());
      diagnose(case_range,
               diagnostic::err_syntax_should_have_at_least_one_executable_statement)
          << "'" << case_token << "' label in a 'switch'";
      return switch_statement;
    }
    if (ast::isa<ast::statement::type::statements>(statements)) {
      ast::statements *code_block = static_cast<ast::statements *>(*statements);
      if (code_block->size() == 0) {
        diagnose(case_token.location(),
                 diagnostic::err_syntax_should_have_at_least_one_executable_statement)
          << "'" << case_token << "' label in a 'switch'";
        return switch_statement;
      }
    }

    cases.emplace_back(case_item_list, statements);
  }

  if (not lexer_.head().is<token::type::r_brace>()) {
    diagnose(lexer_.head().location().start(), diagnostic::err_expected_token_at)
        << token(token::type::r_brace, U"}", location(), location())
        << "end of 'switch' statement";
    return switch_statement;
  }
  lexer_.next();

  switch_statement =
      semantic_analyzer_.switch_statement(control_expression, cases);
  return switch_statement;
}

// labeled-statement → statement-label loop-statement
// labeled-statement → statement-label if-statement
// labeled-statement → statement-label switch-statement
//
// statement-label → label-name ':'
//
// label-name → identifier
parse::result<ast::statement> parser::parse_labelled_statement() {
  assert((lexer_.head().is<token::type::identifier>() and
          lexer_.peek().is<token::type::colon>()) &&
         "expected label");

  parse::result<ast::statement> body;

  token label_name = lexer_.next();
  lexer_.next();

  switch (lexer_.head()) {
  case token::type::kw_switch:
    if (not (body = parse_switch_statement()))
      return body;
    break;
  case token::type::kw_for:
  case token::type::kw_while:
  case token::type::kw_repeat:
    if (not (body = parse_loop_statement()))
      return body;
    break;
  default:
    __builtin_trap(); // TODO(compnerd) diagnose
  }

  body = semantic_analyzer_.labelled_statement(label_name.value(), body);
  return body;
}

// control-transfer-statement → break-statement
// control-transfer-statement → continue-statement
// control-transfer-statement → fallthrough-statement
// control-transfer-statement → return-statement
parse::result<ast::statement> parser::parse_control_transfer_statement() {
  assert((lexer_.head().is<token::type::kw_break>() or
          lexer_.head().is<token::type::kw_continue>() or
          lexer_.head().is<token::type::kw_fallthrough>() or
          lexer_.head().is<token::type::kw_return>()) &&
         "expected 'break', 'continue', 'fallthrough', or 'return'");

  switch (lexer_.head()) {
  default: swift_unreachable("invalid control transfer statement");
  case token::type::kw_break:
    return parse_break_statement();
  case token::type::kw_continue:
    return parse_continue_statement();
  case token::type::kw_fallthrough:
    return parse_fallthrough_statement();
  case token::type::kw_return:
    return parse_return_statement();
  }
}

// break-statement → 'break' label-name[opt]
parse::result<ast::statement> parser::parse_break_statement() {
  assert(lexer_.head().is<token::type::kw_break>() && "expected 'break'");

  std::u32string_view label_name;
  parse::result<ast::statement> break_statement;

  lexer_.next();

  if (lexer_.head().is<token::type::identifier>())
    label_name = lexer_.next().value();

  break_statement = semantic_analyzer_.break_statement(label_name);
  return break_statement;
}

// continue-statement → 'continue' label-name[opt]
parse::result<ast::statement> parser::parse_continue_statement() {
  assert(lexer_.head().is<token::type::kw_continue>() && "expected 'continue'");

  std::u32string_view label_name;
  parse::result<ast::statement> continue_statement;

  lexer_.next();

  if (lexer_.head().is<token::type::identifier>())
    label_name = lexer_.next().value();

  continue_statement = semantic_analyzer_.continue_statement(label_name);
  return continue_statement;
}

// fallthrough-statement → 'fallthrough'
parse::result<ast::statement> parser::parse_fallthrough_statement() {
  assert(lexer_.head().is<token::type::kw_fallthrough>() &&
         "expected 'fallthrough'");
  __builtin_trap();
}

// return-statement → 'return' expression[opt]
parse::result<ast::statement> parser::parse_return_statement() {
  assert(lexer_.head().is<token::type::kw_return>() && "expected 'return'");

  token return_token = lexer_.next();
  parse::result<ast::statement> return_statement;
  return_statement =
      semantic_analyzer_.return_statement(return_token.location().start(),
                                          parse_expression());
  return return_statement;
}

// defer-statement → 'defer' code-block
parse::result<ast::statement> parser::parse_defer_statement() {
  assert(lexer_.head().is<token::type::kw_defer>() && "expected 'defer'");

  token defer_token = lexer_.next();
  parse::result<ast::statement> defer_statement;
  defer_statement =
      semantic_analyzer_.defer_statement(defer_token.location().start(),
                                         parse_code_block());
  return defer_statement;
}

// do-statement → 'do' code-block catch-clauses[opt]
// catch-clauses → catch-clause catch-clauses[opt]
// catch-clause → 'catch' pattern[opt] where-clause[opt] code-block
// where-clause → 'where' where-expression
// where-expression → expression
parse::result<ast::statement> parser::parse_do_statement() {
  assert(lexer_.head().is<token::type::kw_do>() && "expected 'do'");
  lexer_.next();
  __builtin_trap();
}

// compiler-control-statement → build-configuration-statement
// compiler-control-statement → line-control-statement
parse::result<ast::statement> parser::parse_compiler_control_statement() {
  assert((lexer_.head().is<token::type::pp_if>() ||
          lexer_.head().is<token::type::pp_line>()) &&
         "expected '#if' or '#line'");

  switch (lexer_.head()) {
  default:
    return parse::result<ast::statement>();
  case token::type::pp_if:
    return parse_build_configuration_statement();
  case token::type::pp_line:
    return parse_line_control_statement();
  }
}

// build-configuration-statement → '#if' ­build-configuration­ statements[­opt] ­build-configuration-elseif-clauses­[opt]­ build-configuration-else-clause­opt­ '#endif­'
//
// build-configuration-elseif-clauses → build-configuration-elseif-clause­ build-configuration-elseif-clauses­[opt­]
// build-configuration-elseif-clause → '#elseif' ­build-configuration­ statements­[opt­]
// build-configuration-else-clause → '#else­' statements­[opt­]
//
// build-configuration → platform-testing-function­
// build-configuration → identifier­
// build-configuration → boolean-literal­
// build-configuration → '(' ­build-configuration­­ ')'
// build-configuration → '!' ­build-configuration­
// build-configuration → build-configuration­ '&&' ­build-configuration­
// build-configuration → build-configuration­ '||' ­build-configuration­
//
// platform-testing-function → 'os' '(' ­operating-system­ ')'­
// platform-testing-function → 'arch' '­(' ­architecture­ ')'­
//
// operating-system → 'OSX­' | 'iOS'­ | 'watchOS­' | 'tvOS­'
//
// architecture → 'i386­' | 'x86_64­' | 'arm­' | 'arm64'
parse::result<ast::statement> parser::parse_build_configuration_statement() {
  assert(lexer_.head().is<token::type::pp_if>() && "expected '#if'");
  __builtin_trap();
}

// line-control-statement → '#line'
// line-control-statement → '#line' line-number file-name
// line-number → A decimal integer greater than zero
// file-name → static-string-literal
parse::result<ast::statement> parser::parse_line_control_statement() {
  assert(lexer_.head().is<token::type::pp_line>() && "expected '#line'");

  parse::result<ast::statement> line_control_statement;

  lexer_.next();
  if (not lexer_.head().is<token::literal_type::integral>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_starting_line_number);
    return line_control_statement;
  }
  auto line_number = lexer_.next();

  if (not lexer_.head().is<token::literal_type::string>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_filename);
    return line_control_statement;
  }
  auto file_name = lexer_.next();

  line_control_statement =
      semantic_analyzer_.line_control_statement(file_name.value(),
                                                line_number.location().start(),
                                                line_number.value());
  return line_control_statement;
}

// attributes → attribute attributes[opt]
bool parser::parse_attributes(bool is_declaration) {
  for (bool parsed = false; ; parsed = true)
    if (not parse_attribute(is_declaration))
      return parsed;
}

// attribute → '@' attribute-name attribute-argument-clause[opt]
// attribute-name → identifier
// attribute-argument-clause → '(' balanced-tokens[opt] ')'
//
// balanced-tokens → balanced-token balanced-tokens[opt]
// balanced-token → '(' balanced-tokens[opt] ')'
// balanced-token → '[' balanced-tokens[opt] ']'
// balanced-token → '{' balanced-tokens[opt] '}'
// balanced-token → any identifier, keyword, literal, or operator
// balanced-token → any punctuation except '(', ')', '[', ']', '{', or '}'
bool parser::parse_attribute(bool is_declaration) {
  enum : uint8_t {
    invalid_attribute = (9 << 0),
    declaration_attribute = (1 << 0),
    type_attribute = (1 << 1),
  };
  static const struct attribute {
    const char32_t *const spelling;
    const uint8_t application;
  } known_attributes[] = {
    { U"autoclosure", declaration_attribute | type_attribute, },

    { U"NSApplicationMain", declaration_attribute, },
    { U"NSCopying", declaration_attribute, },
    { U"NSManaged", declaration_attribute, },
    { U"UIApplicationMain", declaration_attribute, },
    { U"availability", declaration_attribute, },
    { U"noreturn", declaration_attribute, },
    { U"objc", declaration_attribute, },
    /* undocumented */
    { U"LLDBDebuggerFunction", declaration_attribute, },
    { U"asmname", declaration_attribute, },
    { U"class_protocol", declaration_attribute, },
    { U"exported", declaration_attribute, },
    { U"requires_stored_property_inits", declaration_attribute, },
    { U"transparent", declaration_attribute, },

    { U"IBAction", type_attribute, },
    { U"IBDesignable", type_attribute, },
    { U"IBInspectable", type_attribute, },
    { U"IBOutlet", type_attribute, },
    /* undocumented */
    { U"autoreleased", type_attribute, },
    { U"cc", type_attribute, },
    { U"inout", type_attribute, },
    { U"objc_block", type_attribute, },
    { U"objc_metatype", type_attribute, },
    { U"opened", type_attribute, },
    { U"sil_self", type_attribute, },
    { U"sil_unmanaged", type_attribute, },
    { U"sil_unowned", type_attribute, },
    { U"sil_weak", type_attribute, },
    { U"thin", type_attribute, },
  };

  if (not lexer_.head().is<token::type::at>())
    return false;
  token at = lexer_.next();

  if (not lexer_.head().is<token::type::identifier>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_an_attribute_name);
    return false;
  }
  token attribute_name = lexer_.next();

  // FIXME(compnerd) is there a better way to handle the attributes?
  const struct attribute *known_attribute = nullptr;
  for (const auto &attribute : known_attributes) {
    if (attribute.spelling == attribute_name.value()) {
      known_attribute = &attribute;
      break;
    }
  }
  if (known_attribute == nullptr) {
    diagnose(attribute_name.location(), diagnostic::err_unknown_attribute)
        << std::u32string(at.value());
    return false;
  }

  if (is_declaration and
      not (known_attribute->application & declaration_attribute)) {
    diagnose(at.location(), diagnostic::err_type_attribute_on_declaration);
    return false;
  }
  if (not is_declaration and
      not (known_attribute->application & type_attribute)) {
    diagnose(at.location(), diagnostic::err_declaration_attribute_on_type);
    return false;
  }

  // FIXME(compnerd) why do we need to peek for a literal?
  if (not (lexer_.head().is<token::type::l_paren>() and
           lexer_.peek().is<token::type::identifier>()))
    return true;

  for (unsigned parenthesis = 1, brackets = 0, braces = 0;
       parenthesis or brackets or braces; lexer_.next()) {
    switch (lexer_.head()) {
    default:
      break;
    case token::type::l_paren:
      ++parenthesis;
      break;
    case token::type::r_paren:
      --parenthesis;
      break;
    case token::type::l_brace:
      ++braces;
      break;
    case token::type::r_brace:
      --braces;
      break;
    case token::type::l_square:
      ++brackets;
      break;
    case token::type::r_square:
      --brackets;
      break;
    }
  }

  if (not lexer_.head().is<token::type::r_paren>()) {
    diagnose(lexer_.head().location().start(),
             diagnostic::err_expected_token_after_syntax)
        << token(token::type::r_paren, U")", location(), location())
        << "name for @" << known_attribute->spelling;
    return false;
  }

  return true;
}

// pattern-initializer-list → pattern-initializer
// pattern-initializer-list → pattern-initializer ',' pattern-initializer-list
bool
parser::parse_pattern_initializer_list(std::vector<ast::declaration *> &list) {
  while (true) {
    parse::result<ast::declaration> initializer = parse_pattern_initializer();
    if (not initializer)
      return not list.empty();
    list.push_back(*initializer);

    if (not lexer_.head().is<token::type::comma>())
      return true;
    lexer_.next();
  }
}

// pattern-initializer → pattern initializer[opt]
parse::result<ast::declaration> parser::parse_pattern_initializer() {
  parse::result<ast::declaration> pattern_initializer;

  parse::result<ast::pattern> named = parse_pattern();
  if (not named)
    return pattern_initializer;

  parse::result<ast::expression> initializer;
  if (lexer_.head().is<token::type::equal>()) {
    swift::token equal = lexer_.head();
    if (not (initializer = parse_initializer())) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_initial_value_after)
          << equal;
      return pattern_initializer;
    }
  }

  pattern_initializer =
      semantic_analyzer_.constant_declaration(named, initializer);
  return pattern_initializer;
}

// pattern → wildcard-pattern type-annotation[opt]
// pattern → identifier-pattern type-annotation[opt]
// pattern → value-binding-pattern
// pattern → tuple-pattern type-annotation[opt]
// pattern → enum-case-pattern
// pattern → type-casting-pattern
// pattern → expression-pattern
//
// wildcard-pattern → '_'
//
// identifier-pattern → identifier
//
// enum-case-pattern → type-identifier[opt] '.' enum-case-name tuple-pattern[opt]
//
// type-casting-pattern → is-pattern
// type-casting-pattern → as-pattern
//
// is-pattern → 'is' type
// as-pattern → pattern 'as' type
parse::result<ast::pattern> parser::parse_pattern() {
  parse::result<ast::pattern> pattern;

  switch (lexer_.head()) {
  default:
    if (parse::result<ast::expression> expression = parse_expression())
      pattern = semantic_analyzer_.pattern_expression(expression);
    else
      __builtin_trap();
    break;
  case token::type::kw_let:
  case token::type::kw_var:
    if (semantic_analyzer_.current_scope().is<semantic::scope::type::pattern>()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_token_cannot_appear_nested_inside_another_pattern)
          << lexer_.head();
      consume_until(token::type::equal);
      return pattern;
    }
    return parse_value_binding_pattern();
  case token::type::identifier: {
    token name = lexer_.next();
    pattern = underscore(name)
                  ? semantic_analyzer_.pattern_any()
                  : semantic_analyzer_.pattern_named(name.value(),
                                                     /*Implicit=*/false);
    break;
  }
  case token::type::l_paren:  // tuple-pattern
    pattern = parse_tuple_pattern();
    break;
  }

  if (not semantic_analyzer_.current_scope()
              .is<semantic::scope::type::switch_case>()) {
    if (lexer_.head().is<token::type::colon>()) {
      if (auto type_annotation = parse_type_annotation())
        pattern = semantic_analyzer_.pattern_typed(pattern, type_annotation);
      else
        pattern.invalidate();
    }
  }

  return pattern;
}

// value-binding-pattern → 'var' pattern
// value-binding-pattern → 'let' pattern
parse::result<ast::pattern> parser::parse_value_binding_pattern() {
  assert((lexer_.head().is<token::type::kw_var>() or
          lexer_.head().is<token::type::kw_let>()) &&
         "expected 'let' or 'var'");
  lexer_.next();

  parse::result<ast::pattern> pattern;
  location pattern_location = lexer_.head().location().start();

  if (not (pattern = parse_pattern())) {
    diagnose(pattern_location, diagnostic::err_expected_pattern);
    return pattern;
  }

  return (pattern = semantic_analyzer_.pattern_var(pattern));
}

// tuple-pattern → '(' tuple-pattern-element-list[opt] ')'
//
// tuple-pattern-element-list → tuple-pattern-element
// tuple-pattern-element-list → tuple-pattern-element ',' tuple-pattern-element-list
//
// tuple-pattern-element → pattern
parse::result<ast::pattern> parser::parse_tuple_pattern() {
  assert(lexer_.head().is<token::type::l_paren>() && "expected '('");

  parse::result<ast::pattern> tuple_pattern;
  std::vector<ast::pattern *> elements;

  lexer_.next();

  while (not lexer_.head().is<token::type::eof>()) {
    if (auto pattern = parse_pattern()) {
      elements.push_back(*pattern);
    } else if (elements.size()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_expected_expression_in)
          << "list of expressions";
      return tuple_pattern;
    }

    if (not lexer_.head().is<token::type::comma>())
      break;
    if (not elements.size()) {
      diagnose(lexer_.head().location().start(),
               diagnostic::err_unexpected_token_separator)
          << lexer_.head();
      return tuple_pattern;
    }
    lexer_.next();
  }

  if (lexer_.head().is<token::type::eof>())
    return tuple_pattern;

  if (not lexer_.head().is<token::type::r_paren>())
    __builtin_trap();  // TODO(compnerd) diagnose missing ')'
  lexer_.next();

  return (tuple_pattern = semantic_analyzer_.pattern_tuple(elements));
}

// initializer → '=' expression
parse::result<ast::expression> parser::parse_initializer() {
  assert(lexer_.head().is<token::type::equal>() && "expected '='");
  lexer_.next();
  return parse_expression();
}

// access-level-modifiers → access-level-modifier access-level-modifiers[opt]
bool parser::parse_access_level_modifiers() {
  for (bool parsed = false; ; parsed = true)
    if (not parse_access_level_modifier())
      return parsed;
}

// access-level-modifier → 'internal'
// access-level-modifier → 'internal' '(' 'set' ')'
// access-level-modifier → 'private'
// access-level-modifier → 'private' '(' 'set' ')'
// access-level-modifier → 'public'
// access-level-modifier → 'public' '(' 'set' ')'
bool parser::parse_access_level_modifier() {
  switch (lexer_.head()) {
  default:
    return false;
  case token::type::kw_internal:
  case token::type::kw_private:
  case token::type::kw_public:
    lexer_.next();
    break;
  }

  if (lexer_.head().is<token::type::l_paren>()) {
    lexer_.next();

    if (not lexer_.head().is<token::type::kw_set>())
      __builtin_trap();
    lexer_.next();

    if (not lexer_.head().is<token::type::r_paren>())
      __builtin_trap();
    lexer_.next();
  }

  return true;
}

// declaration-modifiers → declaration-modifier declaration-modifiers[opt]
bool parser::parse_declaration_modifiers() {
  for (bool parsed = false; ; parsed = true)
    if (not parse_declaration_modifier())
      return parsed;
}

// declaration-modifier → 'class'
// declaration-modifier → 'convenience'
// declaration-modifier → 'dynamic'
// declaration-modifier → 'final'
// declaration-modifier → 'infix'
// declaration-modifier → 'lazy'
// declaration-modifier → 'mutating'
// declaration-modifier → 'nonmutating'
// declaration-modifier → 'optional'
// declaration-modifier → 'override'
// declaration-modifier → 'postfix
// declaration-modifier → 'prefix'
// declaration-modifier → 'required'
// declaration-modifier → 'static'
// declaration-modifier → 'unowned'
// declaration-modifier → 'unowned' '(' 'safe' ')'
// declaration-modifier → 'unowned' '(' 'unsafe' ')'
// declaration-modifier → 'weak'
// declaration-modifier → access-level-modifier
bool parser::parse_declaration_modifier() {
  switch (lexer_.head()) {
  default:
    return parse_access_level_modifier();
  case token::type::kw_class:
    if (lexer_.peek().is<token::type::identifier>() and
        not lexer_.peek().is<token::type::kw_func>())
      return false;
  case token::type::kw_convenience:
  case token::type::kw_dynamic:
  case token::type::kw_final:
    lexer_.next();
    break;
  case token::type::kw_infix:
    if (lexer_.peek().is<token::type::kw_func>()) {
      diagnose(lexer_.head().location(),
               diagnostic::err_token_modifier_is_not_required_or_allowed_on_func_declarations)
          << lexer_.head();
      return false;
    }
  case token::type::kw_lazy:
  case token::type::kw_mutating:
  case token::type::kw_nonmutating:
  case token::type::kw_optional:
  case token::type::kw_override:
  case token::type::kw_postfix:
  case token::type::kw_prefix:
  case token::type::kw_required:
  case token::type::kw_static:
    lexer_.next();
    break;
  case token::type::kw_unowned:
    __builtin_trap();
  case token::type::kw_weak:
    lexer_.next();
    break;
  }

  return true;
}
}

