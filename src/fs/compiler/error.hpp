#pragma once

#include "fs/lang/types.hpp"

#include <variant>
#include <vector>
#include <optional>
#include <memory>

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

struct invalid_amount_of_arguments
{
	int expected;
	int actual;
	lang::position_tag place_of_arguments;
};

struct type_mismatch
{
	lang::object_type expected_type;
	lang::object_type actual_type;
	lang::position_tag place_of_expression;
};

struct failed_constructor_call;

struct no_matching_constructor_found
{
	lang::object_type attempted_type_to_construct;
	std::vector<std::optional<lang::object_type>> supplied_types;
	lang::position_tag place_of_function_call;
	std::vector<struct unmatched_function_call> errors;
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

struct condition_redefinition
{
	lang::position_tag place_of_redefinition;
	lang::position_tag place_of_original_definition;
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
	invalid_amount_of_arguments,
	type_mismatch,
	failed_constructor_call,
	no_matching_constructor_found,
	nested_arrays_not_allowed,
	non_homogeneous_array,
	empty_socket_group,
	illegal_characters_in_socket_group,
	invalid_socket_group,
	invalid_minimap_icon_size,
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

struct failed_constructor_call
{
	lang::object_type attempted_type_to_construct;
	std::vector<lang::object_type> ctor_argument_types;
	lang::position_tag place_of_function_call;
	/*
	 * we can not hold error_variant directly because this type is already
	 * one of error_variant variants (infinite memory would be needed)
	 * so allocate on the heap instead
	 *
	 * also, we can not use a simpler version of error_variant that does not contain
	 * complex error types because it is pretty much impossible to prove all invariants
	 * of this in the program - there are simply too many intermediate functions which
	 * thanks to recursion can not really prove they will be always a base case that
	 * results in simpler error type
	 */
	std::unique_ptr<error_variant> error;
};

struct unmatched_function_call
{
	std::vector<lang::object_type> expected_argument_types;
	error_variant error;
};


}
