#pragma once

#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/log/logger.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>

#include <variant>

namespace fs::parser {

class parse_metadata;

}

namespace fs::compiler::errors {

struct name_already_exists
{
	lang::position_tag duplicated_name;
	lang::position_tag original_name;
};

struct no_such_name
{
	lang::position_tag name;
};

struct invalid_amount_of_arguments
{
	int min_allowed;
	boost::optional<int> max_allowed;
	int actual;
	lang::position_tag arguments;
};

struct invalid_integer_value
{
	int min_allowed_value;
	boost::optional<int> max_allowed_value;
	int actual_value;
	lang::position_tag origin;
};

struct type_mismatch
{
	lang::object_type expected_type;
	lang::object_type actual_type;
	lang::position_tag expression;
};

struct invalid_ranged_strings_condition
{
	lang::position_tag condition_origin;
	boost::optional<lang::position_tag> comparison_origin;
	boost::optional<lang::position_tag> integer_origin;
};

struct illegal_character_in_socket_spec
{
	lang::position_tag origin;
	int character_index;
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
	lang::position_tag redefinition;
	lang::position_tag original_definition;
};

// this is only for real filters
// spirit filters allow to override actions in nested blocks
struct action_redefinition
{
	lang::position_tag redefinition;
	lang::position_tag original_definition;
};

struct lower_bound_redefinition
{
	lang::position_tag redefinition;
	lang::position_tag original_definition;
};

struct upper_bound_redefinition
{
	lang::position_tag redefinition;
	lang::position_tag original_definition;
};

struct invalid_action
{
	const char* action_name;
	lang::position_tag origin;
};

struct invalid_set_alert_sound
{
	lang::position_tag sequence_origin;
};

struct price_without_autogen
{
	lang::position_tag visibility_origin;

	// price is a range-based condition so we allow
	// an optional second origin to be specified
	lang::position_tag price_origin;
	boost::optional<lang::position_tag> another_price_origin;
};

enum class autogen_error_cause
{
	expected_empty_condition,
	invalid_rarity_condition,
	invalid_class_condition
};

struct autogen_error
{
	autogen_error_cause cause;
	lang::position_tag autogen_origin;
	lang::position_tag condition_origin;
	lang::position_tag visibility_origin;
};

BETTER_ENUM(internal_compiler_error_cause, int,
	evaluate_socket_spec_literal,

	add_boolean_condition,
	add_range_condition,
	add_numeric_comparison_condition,
	add_string_array_condition,
	add_ranged_string_array_condition,

	real_filter_add_color_action,

	spirit_filter_add_set_color_action_impl,
	spirit_filter_verify_autogen_conditions
)

struct internal_compiler_error
{
	internal_compiler_error_cause cause;
	lang::position_tag origin;
};

} // namespace fs::compiler:errors

namespace fs::compiler::warnings
{

struct font_size_outside_range
{
	lang::position_tag origin;
};

} // namespace fs::compiler::warnings

namespace fs::compiler::notes
{

struct fixed_text
{
	const char* text;
};

struct failed_operations_count
{
	const char* operation_name;
	int count;
};

} // namespace fs::compiler::notes

namespace fs::compiler
{

using error = std::variant<
	errors::name_already_exists,
	errors::no_such_name,
	errors::invalid_amount_of_arguments,
	errors::invalid_integer_value,
	errors::type_mismatch,
	errors::invalid_ranged_strings_condition,
	errors::illegal_character_in_socket_spec,
	errors::invalid_socket_spec,
	errors::duplicate_influence,
	errors::condition_redefinition,
	errors::action_redefinition,
	errors::lower_bound_redefinition,
	errors::upper_bound_redefinition,
	errors::invalid_action,
	errors::invalid_set_alert_sound,
	errors::price_without_autogen,
	errors::autogen_error,
	errors::internal_compiler_error
>;

using warning = std::variant<
	warnings::font_size_outside_range
>;

using note = std::variant<
	notes::fixed_text,
	notes::failed_operations_count
>;

using diagnostic_message = std::variant<error, warning, note>;
using diagnostics_container = boost::container::small_vector<diagnostic_message, 4>;

inline bool has_errors(const diagnostics_container& messages)
{
	for (const auto& msg : messages)
		if (std::holds_alternative<error>(msg))
			return true;

	return false;
}

inline bool has_warnings(const diagnostics_container& messages)
{
	for (const auto& msg : messages)
		if (std::holds_alternative<warning>(msg))
			return true;

	return false;
}

inline bool has_warnings_or_errors(const diagnostics_container& messages)
{
	return has_warnings(messages) || has_errors(messages);
}

void output_diagnostics(
	const diagnostics_container& messages,
	const parser::parse_metadata& metadata,
	log::logger& logger);

} // namespace fs::compiler
