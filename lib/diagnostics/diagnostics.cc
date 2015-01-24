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

#include "swift/diagnostics/diagnostics.hh"

using namespace swift::diagnostics;

static const struct {
  diagnostic::level level;
  const char *format;
} diagnostics[] = {
  [static_cast<int>(diagnostic::err_additional_case_blocks_cannot_appear_after_default)] = { diagnostic::level::error, "additional 'case' blocks cannot appear after 'default' block of a 'switch'" },
  [static_cast<int>(diagnostic::err_cannot_create_variadic_tuple)] = { diagnostic::level::error, "cannot create variadic tuple" },
  [static_cast<int>(diagnostic::err_cannot_declare_a_custom_prefix_name_operator)] = { diagnostic::level::error, "cannot declare a custom prefix '%0' operator" },
  [static_cast<int>(diagnostic::err_cannot_declare_a_custom_postfix_name_operator)] = { diagnostic::level::error, "cannot declare a custom prefix '%0' operator" },
  [static_cast<int>(diagnostic::err_declaration_is_only_valid_at_file_scope)] = { diagnostic::level::error, "declaration is only valid at file scope" },
  [static_cast<int>(diagnostic::err_declaration_attribute_on_type)] = { diagnostic::level::error, "attribute can only be applied to declarations, not types" },
  [static_cast<int>(diagnostic::err_expected_an_attribute_name)] = { diagnostic::level::error, "expected an attribute name" },
  [static_cast<int>(diagnostic::err_expected_argument_list)] = { diagnostic::level::error, "expected argument list" },
  [static_cast<int>(diagnostic::err_expected_dictionary_value_type)] = { diagnostic::level::error, "expected dictionary value type" },
  [static_cast<int>(diagnostic::err_expected_element_type)] = { diagnostic::level::error, "expected element type" },
  [static_cast<int>(diagnostic::err_expected_expression)] = { diagnostic::level::error, "expected expression" },
  [static_cast<int>(diagnostic::err_expected_expression_after_operator)] = { diagnostic::level::error, "expected expression after operator" },
  [static_cast<int>(diagnostic::err_expected_expression_after_syntax)] = { diagnostic::level::error, "expected expression after %0" },
  [static_cast<int>(diagnostic::err_expected_expression_after_token_in)] = { diagnostic::level::error, "expected expression after '%0' in %1" },
  [static_cast<int>(diagnostic::err_expected_expression_for_syntax)] = { diagnostic::level::error, "expected expression for %0" },
  [static_cast<int>(diagnostic::err_expected_expression_in)] = { diagnostic::level::error, "expected expression in %0" },
  [static_cast<int>(diagnostic::err_expected_expression_or_binding_in)] = { diagnostic::level::error, "expected expression, var, or let in %0" },
  [static_cast<int>(diagnostic::err_expected_filename)] = { diagnostic::level::error, "expected filename string literal for #line directive" },
  [static_cast<int>(diagnostic::err_expected_identifier_after_syntax)] = { diagnostic::level::error, "expected identifier after %0" },
  [static_cast<int>(diagnostic::err_expected_identifier)] = { diagnostic::level::error, "expected identifer" },
  [static_cast<int>(diagnostic::err_expected_identifier_in)] = { diagnostic::level::error, "expected identifier in %0" },
  [static_cast<int>(diagnostic::err_expected_identifier_or_token_after_syntax)] = { diagnostic::level::error, "expected identifier or '%0' after %1" },
  [static_cast<int>(diagnostic::err_expected_identifier_for_type_name)] = { diagnostic::level::error, "expected identifier for type name" },
  [static_cast<int>(diagnostic::err_expected_initial_value_after)] = { diagnostic::level::error, "expected initial value after '%0'" },
  [static_cast<int>(diagnostic::err_expected_initialization_in)] = { diagnostic::level::error, "expected initialization in %1" },
  [static_cast<int>(diagnostic::err_expected_member_name_following_token)] = { diagnostic::level::error, "expected member name following '%0'" },
  [static_cast<int>(diagnostic::err_expected_operator_name_in_operator_declaration)] = { diagnostic::level::error, "expected operator name in operator declaration" },
  [static_cast<int>(diagnostic::err_expected_parameter_name)] = { diagnostic::level::error, "expected parameter name" },
  [static_cast<int>(diagnostic::err_expected_parameter_type_following)] = { diagnostic::level::error, "expected parameter type following '%0'" },
  [static_cast<int>(diagnostic::err_expected_pattern)] = { diagnostic::level::error, "expected pattern" },
  [static_cast<int>(diagnostic::err_expected_starting_line_number)] = { diagnostic::level::error, "expected starting line number for #line directive" },
  [static_cast<int>(diagnostic::err_expected_token)] = { diagnostic::level::error, "expected '%0'" },
  [static_cast<int>(diagnostic::err_expected_token_after_syntax)] = { diagnostic::level::error, "expected '%0' after %1" },
  [static_cast<int>(diagnostic::err_expected_token_after_token)] = { diagnostic::level::error, "expected '%0' after '%1'" },
  [static_cast<int>(diagnostic::err_expected_token_at)] = { diagnostic::level::error, "expected '%0' at %1" },
  [static_cast<int>(diagnostic::err_expected_token_before_syntax)] = { diagnostic::level::error, "expected '%0' before %1" },
  [static_cast<int>(diagnostic::err_expected_token_in)] = { diagnostic::level::error, "expected '%0' in %1" },
  [static_cast<int>(diagnostic::err_expected_token_or_token_after_token)] = { diagnostic::level::error, "expected '%0' or '%1' after '%2'" },
  [static_cast<int>(diagnostic::err_expected_token_to_syntax)] = { diagnostic::level::error, "expected '%0' to %1" },
  [static_cast<int>(diagnostic::err_expected_token_type)] = { diagnostic::level::error, "expected '%0' %1" },
  [static_cast<int>(diagnostic::err_expected_type)] = { diagnostic::level::error, "expected type" },
  [static_cast<int>(diagnostic::err_expected_type_in)] = { diagnostic::level::error, "expected type in %0" },
  [static_cast<int>(diagnostic::err_expected_type_for_function_result)] = { diagnostic::level::error, "expected type for function result" },
  [static_cast<int>(diagnostic::err_expected_typed_expression_for)] = { diagnostic::level::error, "expected %0 expression for %1" },
  [static_cast<int>(diagnostic::err_expected_value_in)] = { diagnostic::level::error, "expected value in %0" },
  [static_cast<int>(diagnostic::err_expressions_not_allowed_at_the_top_level)] = { diagnostic::level::error, "expressions not allowed at the top level" },
  [static_cast<int>(diagnostic::err_initializer_cannot_be_referenced_without_arguments)] = { diagnostic::level::error, "initializer cannot be referenced without arguments" },
  [static_cast<int>(diagnostic::err_invalid_character_in_source_file)] = { diagnostic::level::error, "invalid character in source file" },
  [static_cast<int>(diagnostic::err_migration_new_array_syntax)] = { diagnostic::level::error, "array types are now written with the brackets around the element type" },
  [static_cast<int>(diagnostic::err_operator_must_be_declared_as_prefix_postfix_or_infix)] = { diagnostic::level::error, "operator must be declared as 'prefix', 'postfix', or 'infix'" },
  [static_cast<int>(diagnostic::err_operators_must_have_one_or_two_arguments)] = { diagnostic::level::error, "operators must have one or two arguments" },
  [static_cast<int>(diagnostic::err_parameter_may_not_have_multiple_specifiers)] = { diagnostic::level::error, "parameter may not have multiple 'inout, 'var', or 'let' specifiers" },
  [static_cast<int>(diagnostic::err_raw_value_for_enum_case_must_be_a_literal)] = { diagnostic::level::error, "raw value for enum case must be a literal" },
  [static_cast<int>(diagnostic::err_return_invalid_outside_of_a_func)] = { diagnostic::level::error, "return invalid outside of a func" },
  [static_cast<int>(diagnostic::err_syntax_is_not_allowed_outside_of_an_enum)] = { diagnostic::level::error, "%0 is not allowed outside of an enum" },
  [static_cast<int>(diagnostic::err_syntax_should_have_at_least_one_executable_statement)] = { diagnostic::level::error, "%0 should have at least one executable statement" },
  [static_cast<int>(diagnostic::err_target_unknown_triple)] = { diagnostic::level::error, "unknown target triple '%0'" },
  [static_cast<int>(diagnostic::err_the_line_number_needs_to_be_greater_than_zero)] = { diagnostic::level::error, "the line number needs to be greater than zero" },
  [static_cast<int>(diagnostic::err_token_cannot_appear_nested_inside_another_pattern)] = { diagnostic::level::error, "'%0' cannot appear nested instead another 'var' or 'let' pattern" },
  [static_cast<int>(diagnostic::err_token_modifier_is_not_required_or_allowed_on_func_declarations)] = { diagnostic::level::error, "'%0' modifier is not required or allowed on func declarations" },
  [static_cast<int>(diagnostic::err_token_must_be_on_the_last_parameter)] = { diagnostic::level::error, "'%0' must be on the last parameter" },
  [static_cast<int>(diagnostic::err_token_requires_a_function_with_an_operator_identifier)] = { diagnostic::level::error, "'%0' requires a function with an operator identifier" },
  [static_cast<int>(diagnostic::err_type_annotation_missing_in)] = { diagnostic::level::error, "type annotation missing in %0" },
  [static_cast<int>(diagnostic::err_type_attribute_on_declaration)] = { diagnostic::level::error, "attribute can only be applied to types, not declarations" },
  [static_cast<int>(diagnostic::err_unexpected_configuration_block_terminator)] = { diagnostic::level::error, "unexpected configuration block terminator" },
  [static_cast<int>(diagnostic::err_unexpected_token_separator)] = { diagnostic::level::error, "unexpected '%0' separator" },
  [static_cast<int>(diagnostic::err_unknown_attribute)] = { diagnostic::level::error, "unknown attribute '%0'" },

  [static_cast<int>(diagnostic::warn_extraneous_token_in)] = { diagnostic::level::warning, "extraneous '%0' in %1" },
  [static_cast<int>(diagnostic::warn_parameter_name_can_be_expression_more_succinctly_as)] = { diagnostic::level::warning, "'%0 %0' can be expressed more succinctly as '#%0'" },

  [static_cast<int>(diagnostic::note_to_match_this_opening_token)] = { diagnostic::level::note, "to match this opening '%0'" },


  [static_cast<int>(diagnostic::warn_unsupported_feature)] = { diagnostic::level::warning, "unsupported feature %0" },
};

namespace swift::diagnostics {
const char *diagnostic::get_format(diagnostic::id id) {
  return ::diagnostics[id].format;
}

diagnostic::level diagnostic::get_level(diagnostic::id id) {
  return ::diagnostics[id].level;
}
}

