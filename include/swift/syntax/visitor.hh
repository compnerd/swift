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

#ifndef swift_syntax_visitor_hh
#define swift_syntax_visitor_hh

#include "swift/support/visitor.hh"

#include "swift/syntax/source-file.hh"
#include "swift/syntax/top-level-declaration.hh"

#include "swift/syntax/statement.hh"

#include "swift/syntax/expression.hh"
#include "swift/syntax/declaration.hh"
#include "swift/syntax/loop-statement.hh"
#include "swift/syntax/branch-statement.hh"
#include "swift/syntax/labelled-statement.hh"
#include "swift/syntax/control-transfer-statement.hh"
#include "swift/syntax/defer-statement.hh"
#include "swift/syntax/do-statement.hh"
#include "swift/syntax/compiler-control-statement.hh"
#include "swift/syntax/statements.hh"

#include "swift/syntax/expression.hh"
#include "swift/syntax/prefix-unary-expression.hh"
#include "swift/syntax/in-out-expression.hh"
#include "swift/syntax/sequence-expression.hh"
#include "swift/syntax/assignment-expression.hh"
#include "swift/syntax/conditional-expression.hh"
#include "swift/syntax/type-casting-expression.hh"
#include "swift/syntax/declaration-reference-expression.hh"
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

#include "swift/syntax/type-casting-expression.hh"
#include "swift/syntax/is-subtype-expression.hh"
#include "swift/syntax/checked-cast-expression.hh"
#include "swift/syntax/conditional-checked-cast-expression.hh"

#include "swift/syntax/literal-expression.hh"
#include "swift/syntax/boolean-literal-expression.hh"
#include "swift/syntax/floating-point-literal-expression.hh"
#include "swift/syntax/integer-literal-expression.hh"
#include "swift/syntax/nil-literal-expression.hh"
#include "swift/syntax/string-literal-expression.hh"
#include "swift/syntax/array-literal-expression.hh"
#include "swift/syntax/dictionary-literal-expression.hh"
#include "swift/syntax/magic-literal-expression.hh"

#include "swift/syntax/declaration.hh"
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

#include "swift/syntax/loop-statement.hh"
#include "swift/syntax/for-statement.hh"
#include "swift/syntax/for-in-statement.hh"
#include "swift/syntax/while-statement.hh"
#include "swift/syntax/repeat-while-statement.hh"

#include "swift/syntax/branch-statement.hh"
#include "swift/syntax/if-statement.hh"
#include "swift/syntax/guard-statement.hh"
#include "swift/syntax/switch-statement.hh"

#include "swift/syntax/control-transfer-statement.hh"
#include "swift/syntax/break-statement.hh"
#include "swift/syntax/continue-statement.hh"
#include "swift/syntax/fallthrough-statement.hh"
#include "swift/syntax/return-statement.hh"
#include "swift/syntax/throw-statement.hh"

#include "swift/syntax/compiler-control-statement.hh"
#include "swift/syntax/build-configuration-statement.hh"
#include "swift/syntax/line-control-statement.hh"

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

#include "swift/support/error-handling.hh"

namespace swift {
namespace ast {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

template <typename VisitorType, typename ReturnType = void,
          bool ConstVisitor = true>
class visitor
    : public ::visitor<ReturnType, ConstVisitor,
                       source_file,
                       top_level_declaration,

                       statement,
                         expression,
                           prefix_unary_expression,
                           in_out_expression,
                           sequence_expression,
                           postfix_unary_expression,
                           assignment_expression,
                           conditional_expression,
                           type_casting_expression,
                             is_subtype_expression,
                             checked_cast_expression,
                             conditional_checked_cast_expression,
                           function_call_expression,
                           initializer_expression,
                           explicit_member_expression,
                           postfix_self_expression,
                           dynamic_type_expression,
                           subscript_expression,
                           forced_value_expression,
                           optional_chaining_expression,
                           declaration_reference_expression,
                           literal_expression,
                             boolean_literal_expression,
                             floating_point_literal_expression,
                             integer_literal_expression,
                             nil_literal_expression,
                             string_literal_expression,
                             array_literal_expression,
                             dictionary_literal_expression,
                             magic_literal_expression,
                           superclass_expression,
                           closure_expression,
                           parenthesized_expression,
                           implicit_member_expression,
                           wildcard_expression,
                         declaration,
                           import_declaration,
                           constant_declaration,
                           variable_declaration,
                           typealias_declaration,
                           function_declaration,
                           enum_declaration,
                           struct_declaration,
                           class_declaration,
                           protocol_declaration,
                           initializer_declaration,
                           deinitializer_declaration,
                           extension_declaration,
                           subscript_declaration,
                           operator_declaration,
                         loop_statement,
                           for_statement,
                           for_in_statement,
                           while_statement,
                           repeat_while_statement,
                         branch_statement,
                           if_statement,
                           guard_statement,
                           switch_statement,
                         labelled_statement,
                         control_transfer_statement,
                           break_statement,
                           continue_statement,
                           fallthrough_statement,
                           return_statement,
                           throw_statement,
                         defer_statement,
                         do_statement,
                         compiler_control_statement,
                           build_configuration_statement,
                           line_control_statement,
                         statements,

                       pattern,
                         pattern_any,
                         pattern_expression,
                         pattern_named,
                         pattern_tuple,
                         pattern_typed,
                         pattern_var,

                       type,
                         type_array,
                         type_composite,
                         type_dictionary,
                         type_function,
                         type_identifier,
                         type_inout,
                         type_metatype,
                         type_tuple> {
protected:
  using statement_reference =
      typename ::visits<statement, ReturnType, ConstVisitor>::reference_type;

  using expression_reference =
      typename ::visits<expression, ReturnType, ConstVisitor>::reference_type;
  using declaration_reference =
      typename ::visits<declaration, ReturnType, ConstVisitor>::reference_type;
  using loop_statement_reference =
      typename ::visits<loop_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using branch_statement_reference =
      typename ::visits<branch_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using labelled_statement_reference =
      typename ::visits<labelled_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using control_transfer_statement_reference =
      typename ::visits<control_transfer_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using defer_statement_reference =
      typename ::visits<defer_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using do_statement_reference =
      typename ::visits<do_statement, ReturnType, ConstVisitor>::reference_type;
  using compiler_control_statement_reference =
      typename ::visits<compiler_control_statement, ReturnType,
                        ConstVisitor>::reference_type;

  using build_configuration_statement_reference =
      typename ::visits<build_configuration_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using line_control_statement_reference =
      typename ::visits<line_control_statement, ReturnType,
                        ConstVisitor>::reference_type;

  using prefix_unary_expression_reference =
      typename ::visits<prefix_unary_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using in_out_expression_reference =
      typename ::visits<in_out_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using sequence_expression_reference =
      typename ::visits<sequence_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using postfix_unary_expression =
      typename ::visits<postfix_unary_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using function_call_expression_reference =
      typename ::visits<function_call_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using initializer_expression_reference =
      typename ::visits<initializer_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using explicit_member_expression_reference =
      typename ::visits<explicit_member_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using postfix_self_expression_reference =
      typename ::visits<postfix_self_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using dynamic_type_expression_reference =
      typename ::visits<dynamic_type_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using subscript_expression_reference =
      typename ::visits<subscript_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using forced_value_expression_reference =
      typename ::visits<forced_value_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using optional_chaining_expression_reference =
      typename ::visits<optional_chaining_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using declaration_reference_expression_reference =
      typename ::visits<declaration_reference_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using literal_expression_reference =
      typename ::visits<literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using superclass_expression_reference =
      typename ::visits<superclass_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using closure_expression_reference =
      typename ::visits<closure_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using parenthesized_expression_reference =
      typename ::visits<parenthesized_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using implicit_member_expression_reference =
      typename ::visits<implicit_member_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using wildcard_expression_reference =
      typename ::visits<wildcard_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using postfix_unary_expression_reference =
      typename ::visits<postfix_unary_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using assignment_expression_reference =
      typename ::visits<assignment_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using conditional_expression_reference =
      typename ::visits<conditional_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using type_casting_expression_reference =
      typename ::visits<type_casting_expression, ReturnType,
                        ConstVisitor>::reference_type;

  using is_subtype_expression_reference =
      typename ::visits<is_subtype_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using checked_cast_expression_reference =
      typename ::visits<checked_cast_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using conditional_checked_cast_expression_reference =
      typename ::visits<conditional_checked_cast_expression, ReturnType,
                        ConstVisitor>::reference_type;

  using boolean_literal_expression_reference =
      typename ::visits<boolean_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using floating_point_literal_expression_reference =
      typename ::visits<floating_point_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using integer_literal_expression_reference =
      typename ::visits<integer_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using nil_literal_expression_reference =
      typename ::visits<nil_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using string_literal_expression_reference =
      typename ::visits<string_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using array_literal_expression_reference =
      typename ::visits<array_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using dictionary_literal_expression_reference =
      typename ::visits<dictionary_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using magic_literal_expression_reference =
      typename ::visits<magic_literal_expression, ReturnType,
                        ConstVisitor>::reference_type;

  using top_level_declaration_reference =
      typename ::visits<top_level_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using import_declaration_reference =
      typename ::visits<import_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using constant_declaration_reference =
      typename ::visits<constant_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using variable_declaration_reference =
      typename ::visits<variable_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using typealias_declaration_reference =
      typename ::visits<typealias_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using function_declaration_reference =
      typename ::visits<function_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using enum_declaration_reference =
      typename ::visits<enum_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using struct_declaration_reference =
      typename ::visits<struct_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using class_declaration_reference =
      typename ::visits<class_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using protocol_declaration_reference =
      typename ::visits<protocol_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using initializer_declaration_reference =
      typename ::visits<initializer_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using deinitializer_declaration_reference =
      typename ::visits<deinitializer_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using extension_declaration_reference =
      typename ::visits<extension_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using subscript_declaration_reference =
      typename ::visits<subscript_declaration, ReturnType,
                        ConstVisitor>::reference_type;
  using operator_declaration_reference =
      typename ::visits<operator_declaration, ReturnType,
                        ConstVisitor>::reference_type;

  using for_statement_reference =
      typename ::visits<for_statement, ReturnType, ConstVisitor>::reference_type;
  using for_in_statement_reference =
      typename ::visits<for_in_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using while_statement_reference =
      typename ::visits<while_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using repeat_while_statement_reference =
      typename ::visits<repeat_while_statement, ReturnType,
                        ConstVisitor>::reference_type;

  using if_statement_reference =
      typename ::visits<if_statement, ReturnType, ConstVisitor>::reference_type;
  using guard_statement_reference =
      typename ::visits<guard_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using switch_statement_reference =
      typename ::visits<switch_statement, ReturnType,
                        ConstVisitor>::reference_type;

  using break_statement_reference =
      typename ::visits<break_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using continue_statement_reference =
      typename ::visits<continue_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using fallthrough_statement_reference =
      typename ::visits<fallthrough_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using return_statement_reference =
      typename ::visits<return_statement, ReturnType,
                        ConstVisitor>::reference_type;
  using throw_statement_reference =
      typename ::visits<throw_statement, ReturnType,
                        ConstVisitor>::reference_type;

  using statements_reference =
      typename ::visits<statements, ReturnType, ConstVisitor>::reference_type;

  using pattern_reference =
      typename ::visits<pattern, ReturnType, ConstVisitor>::reference_type;
  using pattern_any_reference =
      typename ::visits<pattern_any, ReturnType, ConstVisitor>::reference_type;
  using pattern_expression_reference =
      typename ::visits<pattern_expression, ReturnType,
                        ConstVisitor>::reference_type;
  using pattern_named_reference =
      typename ::visits<pattern_named, ReturnType, ConstVisitor>::reference_type;
  using pattern_tuple_reference =
      typename ::visits<pattern_tuple, ReturnType, ConstVisitor>::reference_type;
  using pattern_typed_reference =
      typename ::visits<pattern_typed, ReturnType, ConstVisitor>::reference_type;
  using pattern_var_reference =
      typename ::visits<pattern_var, ReturnType, ConstVisitor>::reference_type;

  using type_reference =
      typename ::visits<type, ReturnType, ConstVisitor>::reference_type;
  using type_array_reference =
      typename ::visits<type_array, ReturnType, ConstVisitor>::reference_type;
  using type_composite_reference =
      typename ::visits<type_composite, ReturnType,
                        ConstVisitor>::reference_type;
  using type_dictionary_reference =
      typename ::visits<type_dictionary, ReturnType,
                        ConstVisitor>::reference_type;
  using type_function_reference =
      typename ::visits<type_function, ReturnType, ConstVisitor>::reference_type;
  using type_identifier_reference =
      typename ::visits<type_identifier, ReturnType,
                        ConstVisitor>::reference_type;
  using type_inout_reference =
      typename ::visits<type_inout, ReturnType, ConstVisitor>::reference_type;
  using type_metatype_reference =
      typename ::visits<type_metatype, ReturnType,
                        ConstVisitor>::reference_type;
  using type_tuple_reference =
      typename ::visits<type_tuple, ReturnType, ConstVisitor>::reference_type;


public:
  using return_type = ReturnType;

  ReturnType visit(typename ::visits<statement, ReturnType,
                                     ConstVisitor>::reference_type) override;

  ReturnType visit(typename ::visits<expression, ReturnType,
                                     ConstVisitor>::reference_type) override;
  ReturnType visit(typename ::visits<declaration, ReturnType,
                                     ConstVisitor>::reference_type) override;
  ReturnType visit(typename ::visits<loop_statement, ReturnType,
                                     ConstVisitor>::reference_type) override;
  ReturnType visit(typename ::visits<branch_statement, ReturnType,
                                     ConstVisitor>::reference_type) override;
  ReturnType visit(typename ::visits<control_transfer_statement, ReturnType,
                                     ConstVisitor>::reference_type) override;
  ReturnType visit(typename ::visits<compiler_control_statement, ReturnType,
                                     ConstVisitor>::reference_type) override;

  ReturnType visit(typename ::visits<type_casting_expression, ReturnType,
                                     ConstVisitor>::reference_type) override;

  ReturnType visit(typename ::visits<literal_expression, ReturnType,
                                     ConstVisitor>::reference_type) override;

  ReturnType visit(typename ::visits<pattern, ReturnType,
                                     ConstVisitor>::reference_type) override;

  ReturnType visit(typename ::visits<type, ReturnType,
                                     ConstVisitor>::reference_type) override;

  ReturnType
  operator()(typename ::visits<statement, ReturnType,
                               ConstVisitor>::reference_type statement) {
    return statement.accept(*this);
  }

  ReturnType
  operator()(typename ::visits<statement, ReturnType,
                               ConstVisitor>::pointer_type statement) {
    return statement ? statement->accept(*this) : ReturnType();
  }
};

#pragma clang diagnostic pop

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(statement_reference statement) {
  switch (statement.type()) {
  case statement::type::expression:
    return visit(static_cast<expression_reference>(statement));
  case statement::type::declaration:
    return visit(static_cast<declaration_reference>(statement));
  case statement::type::loop_statement:
    return visit(static_cast<loop_statement_reference>(statement));
  case statement::type::branch_statement:
    return visit(static_cast<branch_statement_reference>(statement));
  case statement::type::labelled_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<labelled_statement_reference>(statement));
  case statement::type::control_transfer_statement:
    return visit(static_cast<control_transfer_statement_reference>(statement));
  case statement::type::defer_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<defer_statement_reference>(statement));
  case statement::type::do_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<do_statement_reference>(statement));
  case statement::type::compiler_control_statement:
    return visit(static_cast<compiler_control_statement_reference>(statement));
  case statement::type::statements:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<statements_reference>(statement));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(expression_reference expression) {
  switch (expression.type()) {
  case expression::type::prefix_unary_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<prefix_unary_expression_reference>(expression));
  case expression::type::in_out_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<in_out_expression_reference>(expression));
  case expression::type::sequence_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<sequence_expression_reference>(expression));
  case expression::type::assignment_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<assignment_expression_reference>(expression));
  case expression::type::conditional_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<conditional_expression_reference>(expression));
  case expression::type::type_casting_expression:
    return visit(static_cast<type_casting_expression_reference>(expression));
  case expression::type::declaration_reference_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<declaration_reference_expression_reference>(expression));
  case expression::type::literal_expression:
    return visit(static_cast<literal_expression_reference>(expression));
  case expression::type::superclass_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<superclass_expression_reference>(expression));
  case expression::type::closure_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<closure_expression_reference>(expression));
  case expression::type::parenthesized_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<parenthesized_expression_reference>(expression));
  case expression::type::implicit_member_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<implicit_member_expression_reference>(expression));
  case expression::type::wildcard_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<wildcard_expression_reference>(expression));
  case expression::type::postfix_unary_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<postfix_unary_expression_reference>(expression));
  case expression::type::function_call_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<function_call_expression_reference>(expression));
  case expression::type::initializer_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<initializer_expression_reference>(expression));
  case expression::type::explicit_member_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<explicit_member_expression_reference>(expression));
  case expression::type::postfix_self_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<postfix_self_expression_reference>(expression));
  case expression::type::dynamic_type_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<dynamic_type_expression_reference>(expression));
  case expression::type::subscript_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<subscript_expression_reference>(expression));
  case expression::type::forced_value_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<forced_value_expression_reference>(expression));
  case expression::type::optional_chaining_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<optional_chaining_expression_reference>(expression));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(declaration_reference declaration) {
  switch (declaration.type()) {
  case declaration::type::top_level_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<top_level_declaration_reference>(declaration));
  case declaration::type::import_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<import_declaration_reference>(declaration));
  case declaration::type::constant_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<constant_declaration_reference>(declaration));
  case declaration::type::variable_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<variable_declaration_reference>(declaration));
  case declaration::type::typealias_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<typealias_declaration_reference>(declaration));
  case declaration::type::function_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<function_declaration_reference>(declaration));
  case declaration::type::enum_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<enum_declaration_reference>(declaration));
  case declaration::type::struct_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<struct_declaration_reference>(declaration));
  case declaration::type::class_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<class_declaration_reference>(declaration));
  case declaration::type::protocol_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<protocol_declaration_reference>(declaration));
  case declaration::type::initializer_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<initializer_declaration_reference>(declaration));
  case declaration::type::deinitializer_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<deinitializer_declaration_reference>(declaration));
  case declaration::type::extension_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<extension_declaration_reference>(declaration));
  case declaration::type::subscript_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<subscript_declaration_reference>(declaration));
  case declaration::type::operator_declaration:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<operator_declaration_reference>(declaration));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(loop_statement_reference loop_statement) {
  switch (loop_statement.type()) {
  case loop_statement::type::for_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<for_statement_reference>(loop_statement));
  case loop_statement::type::for_in_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<for_in_statement_reference>(loop_statement));
  case loop_statement::type::while_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<while_statement_reference>(loop_statement));
  case loop_statement::type::repeat_while_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<repeat_while_statement_reference>(loop_statement));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(branch_statement_reference branch_statement) {
  switch (branch_statement.type()) {
  case branch_statement::type::if_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<if_statement_reference>(branch_statement));
  case branch_statement::type::guard_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<guard_statement_reference>(branch_statement));
  case branch_statement::type::switch_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<switch_statement_reference>(branch_statement));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(control_transfer_statement_reference control_transfer_statement) {
  switch (control_transfer_statement.type()) {
  case control_transfer_statement::type::break_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<break_statement_reference>(control_transfer_statement));
  case control_transfer_statement::type::continue_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<continue_statement_reference>(control_transfer_statement));
  case control_transfer_statement::type::fallthrough_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<fallthrough_statement_reference>(control_transfer_statement));
  case control_transfer_statement::type::return_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<return_statement_reference>(control_transfer_statement));
  case control_transfer_statement::type::throw_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<throw_statement_reference>(control_transfer_statement));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(compiler_control_statement_reference compiler_control_statement) {
  switch (compiler_control_statement.type()) {
  case compiler_control_statement::type::build_configuration_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<build_configuration_statement_reference>(compiler_control_statement));
  case compiler_control_statement::type::line_control_statement:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<line_control_statement_reference>(compiler_control_statement));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType
visitor<VisitorType, ResultType, ConstVisitor>::visit(pattern_reference pattern) {
  switch (pattern.type()) {
  case pattern::type::any:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<pattern_any_reference>(pattern));
  case pattern::type::expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<pattern_expression_reference>(pattern));
  case pattern::type::named:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<pattern_named_reference>(pattern));
  case pattern::type::tuple:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<pattern_tuple_reference>(pattern));
  case pattern::type::typed:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<pattern_typed_reference>(pattern));
  case pattern::type::var:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<pattern_var_reference>(pattern));
  }
}

template <typename VisitorType, typename ResultType, bool ConstVisitor>
ResultType visitor<VisitorType, ResultType, ConstVisitor>::visit(type_casting_expression_reference type_cast) {
  switch (type_cast.type()) {
  case type_casting_expression::type::is_subtype_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<is_subtype_expression_reference>(type_cast));
  case type_casting_expression::type::checked_cast_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<checked_cast_expression_reference>(type_cast));
  case type_casting_expression::type::conditional_checked_cast_expression:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<conditional_checked_cast_expression_reference>(type_cast));
  }
}

template <typename VisitorType, typename ReturnType, bool ConstVisitor>
ReturnType
visitor<VisitorType, ReturnType, ConstVisitor>::visit(literal_expression_reference literal_expression) {
  switch (literal_expression.type()) {
  case literal_expression::type::boolean_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<boolean_literal_expression_reference>(literal_expression));
  case literal_expression::type::floating_point_literal:
    return static_cast<VisitorType &>(*this).visit(
        static_cast<floating_point_literal_expression_reference>(literal_expression));
  case literal_expression::type::integer_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<integer_literal_expression_reference>(literal_expression));
  case literal_expression::type::nil_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<nil_literal_expression_reference>(literal_expression));
  case literal_expression::type::string_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<string_literal_expression_reference>(literal_expression));
  case literal_expression::type::array_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<array_literal_expression_reference>(literal_expression));
  case literal_expression::type::dictionary_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<dictionary_literal_expression_reference>(literal_expression));
  case literal_expression::type::magic_literal:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<magic_literal_expression_reference>(literal_expression));
  }
}

template <typename VisitorType, typename ReturnType, bool ConstVisitor>
ReturnType
visitor<VisitorType, ReturnType, ConstVisitor>::visit(type_reference type) {
  switch (type.kind()) {
  case type::kind::array:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_array_reference>(type));
  case type::kind::composite:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_composite_reference>(type));
  case type::kind::dictionary:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_dictionary_reference>(type));
  case type::kind::function:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_function_reference>(type));
  case type::kind::identifier:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_identifier_reference>(type));
  case type::kind::inout:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_inout_reference>(type));
  case type::kind::metatype:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_metatype_reference>(type));
  case type::kind::tuple:
    return static_cast<VisitorType &>(*this)
        .visit(static_cast<type_tuple_reference>(type));
  }
}
}
}

#endif

