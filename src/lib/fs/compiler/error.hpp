#pragma once

#include "fs/lang/types.hpp"

#include <variant>
#include <vector>
#include <optional>
#include <memory>

// all possible compilation errors

namespace fs::compiler::errors
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

struct index_out_of_range
{
	lang::position_tag place_of_subscript;
	int requested_index;
	int array_size;
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
	lang::position_tag place_of_original_definition;
};

struct upper_bound_redefinition
{
	lang::position_tag place_of_redefinition;
	lang::position_tag place_of_original_definition;
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

}

namespace fs::compiler
{

using compile_error = std::variant<
	errors::name_already_exists,
	errors::no_such_name,
	errors::no_such_function,
	errors::no_such_query,
	errors::invalid_amount_of_arguments,
	errors::type_mismatch,
	errors::failed_constructor_call,
	errors::no_matching_constructor_found,
	errors::nested_arrays_not_allowed,
	errors::non_homogeneous_array,
	errors::index_out_of_range,
	errors::empty_socket_group,
	errors::illegal_characters_in_socket_group,
	errors::invalid_socket_group,
	errors::invalid_minimap_icon_size,
	errors::condition_redefinition,
	errors::lower_bound_redefinition,
	errors::upper_bound_redefinition,
	errors::internal_compiler_error_during_action_evaluation,
	errors::internal_compiler_error_during_range_evaluation,
	errors::internal_compiler_error_during_comparison_condition_evaluation,
	errors::internal_compiler_error_during_string_condition_evaluation,
	errors::internal_compiler_error_during_boolean_condition_evaluation
>;

namespace errors
{

struct failed_constructor_call
{
	lang::object_type attempted_type_to_construct;
	std::vector<lang::object_type> ctor_argument_types;
	lang::position_tag place_of_function_call;
	/*
	 * we can not hold compile_error directly because this type is already
	 * one of compile_error variants (infinite memory would be needed)
	 * so allocate on the heap instead
	 *
	 * also, we can not use a simpler version of compile_error that does not contain
	 * complex error types because it is pretty much impossible to prove all invariants
	 * of this in the program - there are simply too many intermediate functions which
	 * thanks to recursion can not really prove they will be always a base case that
	 * results in simpler error type
	 */
	std::unique_ptr<compile_error> error;
};

struct unmatched_function_call
{
	std::vector<lang::object_type> expected_argument_types;
	compile_error error;
};

} // namespace errors

} // namespace fs::compiler
