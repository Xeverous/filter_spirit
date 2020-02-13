#pragma once

#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>

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

struct action_redefinition
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

struct rule_already_specified
{
	lang::position_tag place;
};

enum class internal_compiler_error_cause
{
	add_boolean_condition,
	add_range_condition,

	real_filter_add_numeric_condition,
	real_filter_add_string_array_condition,
	real_filter_add_color_action,

	spirit_filter_add_array_condition,
	spirit_filter_add_comparison_condition,

	spirit_filter_add_unary_action
};

struct internal_compiler_error
{
	internal_compiler_error_cause cause;
	lang::position_tag origin;
};

}

namespace fs::compiler
{

using compile_error = std::variant<
	errors::name_already_exists,
	errors::no_such_name,
	errors::no_such_query,
	errors::invalid_amount_of_arguments,
	errors::type_mismatch,
	errors::empty_socket_group,
	errors::illegal_characters_in_socket_group,
	errors::invalid_socket_group,
	errors::invalid_minimap_icon_size,
	errors::condition_redefinition,
	errors::action_redefinition,
	errors::lower_bound_redefinition,
	errors::upper_bound_redefinition,
	errors::internal_compiler_error
>;

} // namespace fs::compiler
