#pragma once
#include "lang/types.hpp"
#include "parser/config.hpp" // TODO reduce dependency, remove
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

struct type_mismatch
{
	lang::object_type expected_type;
	lang::object_type actual_type;
	lang::position_tag place_of_expression;
};

struct type_mismatch_in_assignment // TODO remove this, we now auto-deduce types
{
	// invariant: left_operand_type != right_operand_type
	lang::object_type left_operand_type;
	lang::object_type right_operand_type;
	parser::range_type right_operand_value_origin;
	// if object was unnamed literal then type origin should point at that literal
	parser::range_type right_operand_type_origin;
};

struct type_mismatch_in_expression
{
	// invariant: expected_type != actual_type
	lang::object_type expected_type;
	lang::object_type actual_type;
	parser::range_type expression_type_origin;
};

// inform the user that '[expr]' was probably intended than 'expr'
struct single_object_to_array_assignment
{
	lang::object_type left_operand_type;
	lang::single_object_type right_operand_type;
	parser::range_type right_operand_expr;
};

struct nested_arrays_not_allowed
{
	lang::position_tag place_of_nested_array_expression;
};

struct non_homogeneous_array
{
	lang::position_tag place_of_first_element;
	lang::position_tag place_of_second_element;
	lang::single_object_type first_element_type;
	lang::single_object_type second_element_type;
};

struct internal_error_while_parsing_constant
// all available backtrack places when error is generated
{
	parser::range_type wanted_name_origin;
	parser::range_type wanted_type_origin;
	parser::range_type expression_type_origin;
	parser::range_type expression_value_origin;
	std::optional<parser::range_type> expression_name_origin;
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
	parser::range_type first_action_origin;
	parser::range_type second_action_origin;
};

/*
 * identical condition types in 1 condition list, eg
 * ItemLevel 10, ItemLevel 11 {
 *     ...
 * }
 */
struct duplicate_condition
{
	parser::range_type first_condition_origin;
	parser::range_type second_condition_origin;
};

using error_variant = std::variant<
	name_already_exists,
	no_such_name,
	no_such_function,
	invalid_amount_of_arguments,
	empty_socket_group,
	invalid_socket_group,
	type_mismatch,
	single_object_to_array_assignment,
	nested_arrays_not_allowed,
	non_homogeneous_array,
	internal_error_while_parsing_constant,
	duplicate_action,
	duplicate_condition
>;

}
