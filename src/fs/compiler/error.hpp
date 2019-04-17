#pragma once
#include "fs/lang/types.hpp"
#include <variant>

// all possible compilation errors

namespace fs::compiler::error
{

struct name_already_exists
{
	lang::position_tag place_of_duplicated_name;
	lang::position_tag place_of_original_name;
};

struct no_such_name
{
	lang::position_tag place_of_name;
};

struct no_such_function
{
	lang::position_tag place_of_name;
};

struct no_such_query
{
	lang::position_tag place_of_name;
};

struct no_such_advanced_query
{
	lang::position_tag place_of_name;
};

struct invalid_amount_of_arguments
{
	int min_expected;
	int max_expected;
	int actual;
	lang::position_tag place_of_arguments;
};

struct type_mismatch
{
	lang::object_type expected_type;
	lang::object_type actual_type;
	lang::position_tag place_of_expression;
};

struct nested_arrays_not_allowed
{
	lang::position_tag place_of_nested_array_expression;
};

struct non_homogeneous_array
{
	lang::position_tag place_of_first_element;
	lang::position_tag place_of_second_element;
	lang::object_type first_element_type;
	lang::object_type second_element_type;
};

struct empty_socket_group
{
	lang::position_tag place_of_socket_group_string;
};

struct illegal_characters_in_socket_group
{
	lang::position_tag place_of_socket_group_string;
};

struct invalid_socket_group
{
	lang::position_tag place_of_socket_group_string;
};

struct invalid_minimap_icon_size
{
	int requested_size;
	lang::position_tag place_of_size_argument;
};

struct positional_sound_not_supported
{
	lang::position_tag place_of_action;
};

struct disable_drop_sound_not_supported
{
	lang::position_tag place_of_action;
};

struct temporary_beam_effect_not_supported
{
	lang::position_tag place_of_2nd_argument;
};

struct condition_redefinition
{
	lang::position_tag place_of_redefinition;
};

struct lower_bound_redefinition
{
	lang::position_tag place_of_redefinition;
};

struct upper_bound_redefinition
{
	lang::position_tag place_of_redefinition;
};

struct exact_comparison_redefinition
{
	lang::position_tag place_of_redefinition;
};

struct exact_comparison_outside_parent_range
{
	lang::position_tag place_of_redefinition;
};

struct internal_compiler_error_during_action_evaluation
{
	lang::position_tag place_of_action;
};

struct internal_compiler_error_during_range_evaluation
{
	lang::position_tag place_of_comparison_condition;
};

struct internal_compiler_error_during_comparison_condition_evaluation
{
	lang::position_tag place_of_comparison_condition;
};

struct internal_compiler_error_during_string_condition_evaluation
{
	lang::position_tag place_of_string_condition;
};

struct internal_compiler_error_during_boolean_condition_evaluation
{
	lang::position_tag place_of_boolean_condition;
};

using error_variant = std::variant<
	name_already_exists,
	no_such_name,
	no_such_function,
	no_such_query,
	no_such_advanced_query,
	invalid_amount_of_arguments,
	type_mismatch,
	nested_arrays_not_allowed,
	non_homogeneous_array,
	empty_socket_group,
	illegal_characters_in_socket_group,
	invalid_socket_group,
	invalid_minimap_icon_size,
	positional_sound_not_supported,
	disable_drop_sound_not_supported,
	temporary_beam_effect_not_supported,
	condition_redefinition,
	lower_bound_redefinition,
	upper_bound_redefinition,
	exact_comparison_redefinition,
	exact_comparison_outside_parent_range,
	internal_compiler_error_during_action_evaluation,
	internal_compiler_error_during_range_evaluation,
	internal_compiler_error_during_comparison_condition_evaluation,
	internal_compiler_error_during_string_condition_evaluation,
	internal_compiler_error_during_boolean_condition_evaluation
>;

}
