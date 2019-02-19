#pragma once
#include "lang/types.hpp"
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

struct invalid_amount_of_arguments
{
	int min_expected;
	int max_expected;
	int actual;
	lang::position_tag place_of_arguments;
};

struct empty_socket_group
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

struct temporary_beam_effect_not_supported
{
	lang::position_tag place_of_2nd_argument;
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

/*
 * identical action types in 1 action list, eg
 * ... {
 *     SetTextColor 101 102 103
 *     SetTextColor 101 102 103 255
 * }
 */
struct duplicate_action
{
	lang::position_tag place_of_first_action;
	lang::position_tag place_of_second_action;
};

/*
 * identical condition types in 1 condition list, eg
 * ItemLevel 10, ItemLevel 11 {
 *     ...
 * }
 */
struct duplicate_condition
{
	lang::position_tag place_of_first_condition;
	lang::position_tag place_of_second_condition;
};

using error_variant = std::variant<
	name_already_exists,
	no_such_name,
	no_such_function,
	invalid_amount_of_arguments,
	empty_socket_group,
	invalid_socket_group,
	invalid_minimap_icon_size,
	temporary_beam_effect_not_supported,
	type_mismatch,
	nested_arrays_not_allowed,
	non_homogeneous_array,
	duplicate_action,
	duplicate_condition
>;

}
