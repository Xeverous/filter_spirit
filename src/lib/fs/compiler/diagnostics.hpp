#pragma once

#include <fs/compiler/settings.hpp>
#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/log/logger.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>

#include <variant>

namespace fs::parser {

struct parse_metadata;

}

namespace fs::compiler {

enum class diagnostic_message_severity
{
	note, warning, error
};

inline diagnostic_message_severity make_warning_severity(error_handling_settings st) noexcept
{
	if (st.treat_warnings_as_errors)
		return diagnostic_message_severity::error;
	else
		return diagnostic_message_severity::warning;
}

enum class diagnostic_message_id
{
	name_already_exists,
	no_such_name,
	invalid_amount_of_arguments,
	invalid_integer_value,
	type_mismatch,
	invalid_ranged_strings_condition,
	illegal_character_in_socket_spec,
	invalid_socket_spec,
	duplicate_influence,
	condition_redefinition,
	action_redefinition,
	lower_bound_redefinition,
	upper_bound_redefinition,
	invalid_alert_sound_id,
	invalid_set_alert_sound,
	invalid_play_effect,
	price_without_autogen,
	autogen_incompatible_condition,
	autogen_missing_condition,
	recursion_limit_reached,
	internal_compiler_error,
	font_size_outside_range,
	failed_operations_count,
	attempt_description,
	minor_note
};

struct diagnostic_message
{
	diagnostic_message_severity severity;
	diagnostic_message_id id; // tests should check this field
	boost::optional<lang::position_tag> origin;
	std::string description; // should not contain line breaks
};

namespace detail {

	inline std::size_t string_length(const char* c_str) noexcept
	{
		return std::strlen(c_str);
	}

	inline std::size_t string_length(const std::string& str) noexcept
	{
		return str.length();
	}

	inline std::size_t string_length(std::string_view sv) noexcept
	{
		return sv.length();
	}

	template <typename... Strings>
	std::string merge_strings(Strings&&... strings)
	{
		std::string result;
		result.reserve((0 + ... + string_length(strings)));
		(result.append(strings), ...);
		return result;
	}

}

template <typename... Strings>
diagnostic_message
make_diagnostic_message(
	diagnostic_message_severity severity,
	diagnostic_message_id id,
	boost::optional<lang::position_tag> origin,
	Strings&&... strings)
{
	return diagnostic_message{severity, id, origin, detail::merge_strings(std::forward<Strings>(strings)...)};
}

template <typename... Strings>
diagnostic_message
make_note(
	diagnostic_message_id id,
	boost::optional<lang::position_tag> origin,
	Strings&&... strings)
{
	return make_diagnostic_message(diagnostic_message_severity::note, id, origin, std::forward<Strings>(strings)...);
}

template <typename... Strings>
diagnostic_message
make_warning(
	diagnostic_message_id id,
	boost::optional<lang::position_tag> origin,
	Strings&&... strings)
{
	return make_diagnostic_message(diagnostic_message_severity::warning, id, origin, std::forward<Strings>(strings)...);
}

template <typename... Strings>
diagnostic_message
make_error(
	diagnostic_message_id id,
	boost::optional<lang::position_tag> origin,
	Strings&&... strings)
{
	return make_diagnostic_message(diagnostic_message_severity::error, id, origin, std::forward<Strings>(strings)...);
}

template <typename... Strings>
inline diagnostic_message
make_note_minor(boost::optional<lang::position_tag> origin, Strings&&... strings)
{
	return make_note(diagnostic_message_id::minor_note, origin, std::forward<Strings>(strings)...);
}

inline diagnostic_message
make_note_first_defined_here(lang::position_tag origin)
{
	return make_note_minor(origin, "first defined here");
}

template <typename... Strings>
diagnostic_message
make_note_attempt_description(Strings&&... strings)
{
	return make_note(diagnostic_message_id::attempt_description, boost::none, std::forward<Strings>(strings)...);
}

using diagnostics_container = boost::container::small_vector<diagnostic_message, 4>;

namespace detail {

	void push_error_bound_redefinition(
		bool is_lower,
		lang::position_tag redefinition,
		lang::position_tag original,
		diagnostics_container& diagnostics);

}

inline void push_error_name_already_exists(
	lang::position_tag duplicate_origin,
	lang::position_tag existing_origin,
	diagnostics_container& diagnostics)
{
	diagnostics.push_back(make_error(diagnostic_message_id::name_already_exists, duplicate_origin, "name already exists"));
	diagnostics.push_back(make_note_first_defined_here(existing_origin));
}
inline void push_error_no_such_name(
	lang::position_tag name_origin,
	diagnostics_container& diagnostics)
{
	diagnostics.push_back(make_error(diagnostic_message_id::no_such_name, name_origin, "no such name has been defined"));
}
void push_error_invalid_integer_value(
	int min,
	int max,
	lang::integer actual,
	diagnostics_container& diagnostics);
void push_error_invalid_amount_of_arguments(
	int expected_min,
	boost::optional<int> expected_max,
	int actual,
	lang::position_tag origin,
	diagnostics_container& diagnostics);
inline void push_error_lower_bound_redefinition(
	lang::position_tag redefinition,
	lang::position_tag original,
	diagnostics_container& diagnostics)
{
	detail::push_error_bound_redefinition(true, redefinition, original, diagnostics);
}
inline void push_error_upper_bound_redefinition(
	lang::position_tag redefinition,
	lang::position_tag original,
	diagnostics_container& diagnostics)
{
	detail::push_error_bound_redefinition(false, redefinition, original, diagnostics);
}
void push_error_type_mismatch(
	lang::object_type expected,
	lang::object_type actual,
	lang::position_tag origin,
	diagnostics_container& diagnostics);
inline void push_error_recursion_limit_reached(
	lang::position_tag origin,
	diagnostics_container& diagnostics)
{
	diagnostics.push_back(make_error(
		diagnostic_message_id::recursion_limit_reached,
		origin,
		"recursion limit reached - you very likely made recursive tree which never ends"));
}
void push_error_internal_compiler_error(
	std::string_view function_name,
	lang::position_tag origin,
	diagnostics_container& diagnostics);
void push_error_autogen_incompatible_condition(
	lang::position_tag autogen_origin,
	lang::position_tag condition_origin,
	std::string required_matching_value,
	diagnostics_container& diagnostics);
void push_error_autogen_missing_condition(
	lang::position_tag visibility_origin,
	lang::position_tag autogen_origin,
	std::string_view missing_condition,
	diagnostics_container& diagnostics);

inline bool has_errors(const diagnostics_container& messages)
{
	for (const auto& msg : messages)
		if (msg.severity == diagnostic_message_severity::error)
			return true;

	return false;
}

inline bool has_warnings(const diagnostics_container& messages)
{
	for (const auto& msg : messages)
		if (msg.severity == diagnostic_message_severity::error)
			return true;

	return false;
}

inline bool has_warnings_or_errors(const diagnostics_container& messages)
{
	for (const auto& msg : messages)
		if (msg.severity == diagnostic_message_severity::warning || msg.severity == diagnostic_message_severity::error)
			return true;

	return false;
}

void output_diagnostics(
	const diagnostics_container& messages,
	const parser::parse_metadata& metadata,
	log::logger& logger);

} // namespace fs::compiler
