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

#ifndef swift_diagnostics_diagnostics_hh
#define swift_diagnostics_diagnostics_hh

namespace swift {
namespace diagnostics {
class diagnostic {
public:
  enum class level {
    ignored,
    note,
    warning,
    error,
    fatal,
  };

  enum id {
    invalid = ~0,

    err_additional_case_blocks_cannot_appear_after_default,
    err_cannot_create_variadic_tuple,
    err_cannot_declare_a_custom_prefix_name_operator,
    err_cannot_declare_a_custom_postfix_name_operator,
    err_declaration_is_only_valid_at_file_scope,
    err_declaration_attribute_on_type,
    err_expected_an_attribute_name,
    err_expected_argument_list,
    err_expected_dictionary_value_type,
    err_expected_element_type,
    err_expected_expression,
    err_expected_expression_after_operator,
    err_expected_expression_after_syntax,
    err_expected_expression_after_token_in,
    err_expected_expression_for_syntax,
    err_expected_expression_in,
    err_expected_expression_or_binding_in,
    err_expected_filename,
    err_expected_identifier_after_syntax,
    err_expected_identifier,
    err_expected_identifier_in,
    err_expected_identifier_or_token_after_syntax,
    err_expected_identifier_for_type_name,
    err_expected_initial_value_after,
    err_expected_initialization_in,
    err_expected_member_name_following_token,
    err_expected_operator_name_in_operator_declaration,
    err_expected_parameter_name,
    err_expected_parameter_type_following,
    err_expected_pattern,
    err_expected_starting_line_number,
    err_expected_token,
    err_expected_token_after_syntax,
    err_expected_token_after_token,
    err_expected_token_at,
    err_expected_token_before_syntax,
    err_expected_token_in,
    err_expected_token_or_token_after_token,
    err_expected_token_to_syntax,
    err_expected_token_type,
    err_expected_type,
    err_expected_type_for_function_result,
    err_expected_type_in,
    err_expected_typed_expression_for,
    err_expected_value_in,
    err_expressions_not_allowed_at_the_top_level,
    err_initializer_cannot_be_referenced_without_arguments,
    err_invalid_character_in_source_file,
    err_migration_new_array_syntax,
    err_operator_must_be_declared_as_prefix_postfix_or_infix,
    err_operators_must_have_one_or_two_arguments,
    err_parameter_may_not_have_multiple_specifiers,
    err_raw_value_for_enum_case_must_be_a_literal,
    err_return_invalid_outside_of_a_func,
    err_syntax_is_not_allowed_outside_of_an_enum,
    err_syntax_should_have_at_least_one_executable_statement,
    err_target_unknown_triple,
    err_the_line_number_needs_to_be_greater_than_zero,
    err_token_cannot_appear_nested_inside_another_pattern,
    err_token_modifier_is_not_required_or_allowed_on_func_declarations,
    err_token_must_be_on_the_last_parameter,
    err_token_requires_a_function_with_an_operator_identifier,
    err_type_annotation_missing_in,
    err_type_attribute_on_declaration,
    err_unexpected_configuration_block_terminator,
    err_unexpected_token_separator,
    err_unknown_attribute,

    warn_extraneous_token_in,
    warn_parameter_name_can_be_expression_more_succinctly_as,

    note_to_match_this_opening_token,

    warn_unsupported_feature,
  };

  static const char *get_format(diagnostic::id id);
  static level get_level(diagnostic::id id);
};
}
}

#endif

