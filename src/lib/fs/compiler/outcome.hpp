#pragma once

#include <fs/compiler/settings.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/utility/outcome.hpp>
#include <fs/log/logger.hpp>

#include <variant>
#include <optional>

namespace fs::compiler::errors
{

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
	std::optional<int> max_allowed;
	int actual;
	lang::position_tag arguments;
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
	lang::position_tag expression;
};

struct empty_socket_spec
{
	lang::position_tag origin;
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

struct invalid_builtin_alert_sound_id
{
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
	std::optional<lang::position_tag> another_price_origin;
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
	add_boolean_condition,
	add_range_condition,
	add_numeric_comparison_condition,
	add_string_array_condition,

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
	errors::empty_socket_spec,
	errors::illegal_character_in_socket_spec,
	errors::invalid_socket_spec,
	errors::duplicate_influence,
	errors::condition_redefinition,
	errors::action_redefinition,
	errors::lower_bound_redefinition,
	errors::upper_bound_redefinition,
	errors::invalid_builtin_alert_sound_id,
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

using log_message = std::variant<error, warning, note>;

constexpr std::size_t outcome_buffer_size = 4;

template <typename... R>
using outcome = utility::outcome<log_message, outcome_buffer_size, R...>;

using log_container = utility::outcome_base<log_message, outcome_buffer_size>::log_container_type;

inline bool has_errors(const log_container& logs)
{
	for (const auto& msg : logs)
		if (std::holds_alternative<error>(msg))
			return true;

	return false;
}

inline bool has_warnings(const log_container& logs)
{
	for (const auto& msg : logs)
		if (std::holds_alternative<warning>(msg))
			return true;

	return false;
}

inline bool has_warnings_or_errors(const log_container& logs)
{
	return has_warnings(logs) || has_errors(logs);
}

inline bool should_continue(error_handling_settings settings, const log_container& logs)
{
	if (settings.stop_on_error) {
		if (has_errors(logs) || (settings.treat_warnings_as_errors && has_warnings_or_errors(logs)))
			return false;
	}

	return true;
}

template <typename... R>
bool should_continue(error_handling_settings settings, const outcome<R...>& o)
{
	if (!o.has_result())
		return false;

	return should_continue(settings, o.logs());
}

void output_logs(
	const log_container& logs,
	const parser::lookup_data& lookup_data,
	log::logger& logger);

} // namespace fs::compiler
