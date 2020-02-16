#pragma once

#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>

#include <variant>
#include <optional>

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

struct invalid_amount_of_arguments
{
	int min_allowed;
	std::optional<int> max_allowed;
	int actual;
	lang::position_tag place_of_arguments;
};

struct invalid_integer_value
{
	int min_allowed_value;
	std::optional<int> max_allowed_value;
	int actual_value;
	lang::position_tag origin;
};

struct type_mismatch
{
	lang::object_type expected_type;
	lang::object_type actual_type;
	lang::position_tag place_of_expression;
};

struct empty_socket_spec
{
	lang::position_tag origin;
};

struct illegal_characters_in_socket_spec
{
	lang::position_tag origin;
};

struct invalid_socket_spec
{
	lang::position_tag origin;
};

struct duplicate_influence
{
	lang::position_tag first_influence;
	lang::position_tag second_influence;
};

struct condition_redefinition
{
	lang::position_tag place_of_redefinition;
	lang::position_tag place_of_original_definition;
};

// this is only for real filters
// spirit filters allow to override actions in nested blocks
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

enum class internal_compiler_error_cause
{
	add_boolean_condition,
	add_range_condition,
	add_numeric_comparison_condition,
	add_string_array_condition,

	real_filter_add_color_action,

	spirit_filter_add_set_color_action_impl,
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
	errors::invalid_amount_of_arguments,
	errors::invalid_integer_value,
	errors::type_mismatch,
	errors::empty_socket_spec,
	errors::illegal_characters_in_socket_spec,
	errors::invalid_socket_spec,
	errors::duplicate_influence,
	errors::condition_redefinition,
	errors::action_redefinition,
	errors::lower_bound_redefinition,
	errors::upper_bound_redefinition,
	errors::internal_compiler_error
>;

} // namespace fs::compiler
