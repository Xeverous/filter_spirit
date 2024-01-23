#pragma once

#include <fs/compiler/settings.hpp>
#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/log/logger.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <iterator>

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
	// expressions/statements
	unknown_statement,
	invalid_statement,
	unknown_expression,
	invalid_expression,
	invalid_operator,

	// type/value failures
	type_mismatch,
	invalid_amount_of_arguments,
	value_out_of_range,
	invalid_socket_spec,
	duplicate_influence,

	// variables
	name_already_exists,
	no_such_name,

	// condition-specific
	condition_redefinition,
	lower_bound_redefinition,
	upper_bound_redefinition,
	condition_after_equality,
	dead_condition,

	// action-specific
	action_redefinition,
	invalid_alert_sound_id,
	invalid_set_alert_sound,
	invalid_play_effect,

	// autogen-specific
	price_without_autogen,
	autogen_without_price,
	autogen_incompatible_condition,
	autogen_forbidden_condition,
	autogen_missing_condition,

	// other
	internal_compiler_error,
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

class diagnostics_store
{
public:
	using container_type = boost::container::small_vector<diagnostic_message, 4>;

	std::size_t size() const
	{
		return m_messages.size();
	}

	const diagnostic_message& operator[](std::size_t n) const
	{
		return m_messages[n];
	}

	bool has_errors() const
	{
		for (const auto& msg : m_messages)
			if (msg.severity == diagnostic_message_severity::error)
				return true;

		return false;
	}

	bool has_warnings() const
	{
		for (const auto& msg : m_messages)
			if (msg.severity == diagnostic_message_severity::error)
				return true;

		return false;
	}

	bool has_warnings_or_errors() const
	{
		for (const auto& msg : m_messages)
			if (msg.severity == diagnostic_message_severity::warning || msg.severity == diagnostic_message_severity::error)
				return true;

		return false;
	}

	bool is_ok(bool treat_warnings_as_errors) const
	{
		if (treat_warnings_as_errors)
			return !has_warnings_or_errors();
		else
			return !has_errors();
	}

	void move_messages_from(diagnostics_store& other)
	{
		std::move(other.m_messages.begin(), other.m_messages.end(), std::back_inserter(m_messages));
		other.m_messages.clear();
	}

	void output_messages(const parser::parse_metadata& metadata, log::logger& logger) const;

	void push_layer(lang::position_tag layer)
	{
		m_layers.push_back(layer);
	}

	void pop_layer()
	{
		if (m_layers.empty())
			push_error_internal_compiler_error(__func__, lang::position_tag{});
		else
			m_layers.pop_back();
	}

	const auto& layers() const
	{
		return m_layers;
	}

	// ---- helper functions ----

	void push_message(diagnostic_message message)
	{
		m_messages.push_back(std::move(message));
	}

	// expressions/statements

	void push_error_unknown_statement(lang::position_tag statement_origin)
	{
		push_message(make_error(diagnostic_message_id::unknown_statement, statement_origin, "unknown statement"));
	}

	void push_error_invalid_statement(lang::position_tag statement_origin, std::string_view explanation)
	{
		push_message(make_error(diagnostic_message_id::invalid_statement, statement_origin, "invalid statement: ", explanation));
	}

	void push_error_unknown_expression(lang::position_tag expression_origin)
	{
		push_message(make_error(diagnostic_message_id::unknown_expression, expression_origin, "unknown expression"));
	}

	void push_error_invalid_expression(lang::position_tag expression_origin, std::string_view explanation)
	{
		push_message(make_error(diagnostic_message_id::invalid_expression, expression_origin, "invalid expression: ", explanation));
	}

	void push_error_invalid_operator(lang::position_tag operator_origin, std::string_view explanation)
	{
		push_message(make_error(diagnostic_message_id::invalid_operator, operator_origin, "invalid operator: ", explanation));
	}

	// type/value failures

	void push_error_type_mismatch(lang::object_type expected, lang::object_type actual, lang::position_tag origin);

	void push_error_invalid_amount_of_arguments(
		int expected_min, boost::optional<int> expected_max, int actual, lang::position_tag origin);
	void push_error_value_out_of_range(int min, int max, lang::integer actual);

	// variables

	void push_error_name_already_exists(lang::position_tag duplicate_origin, lang::position_tag existing_origin)
	{
		push_message(make_error(diagnostic_message_id::name_already_exists, duplicate_origin, "name already exists"));
		push_message(make_note_first_defined_here(existing_origin));
	}

	void push_error_no_such_name(lang::position_tag name_origin)
	{
		push_message(make_error(diagnostic_message_id::no_such_name, name_origin, "no such name has been defined"));
	}

	// condition-specific

	void push_error_condition_redefinition(lang::position_tag redefinition, lang::position_tag original)
	{
		push_message(make_error(
			diagnostic_message_id::condition_redefinition,
			redefinition,
			"condition redefinition: this condition can not be repeated in the same block or nested blocks"));
		push_message(make_note_first_defined_here(original));
	}

	void push_error_lower_bound_redefinition(lang::position_tag redefinition, lang::position_tag original)
	{
		push_error_bound_redefinition(true, redefinition, original);
	}

	void push_error_upper_bound_redefinition(lang::position_tag redefinition, lang::position_tag original)
	{
		push_error_bound_redefinition(false, redefinition, original);
	}

	void push_error_condition_after_equality(lang::position_tag redefinition, lang::position_tag original)
	{
		push_message(make_error(
			diagnostic_message_id::condition_after_equality,
			redefinition,
			"range/equality specified for a condition that already has an equality requirement"));
		push_message(make_note_first_defined_here(original));
	}

	void push_warning_dead_condition(lang::position_tag condition_origin, std::string_view explanation)
	{
		push_message_dead_condition(diagnostic_message_severity::warning, condition_origin, explanation);
	}

	void push_error_dead_condition(lang::position_tag condition_origin, std::string_view explanation)
	{
		push_message_dead_condition(diagnostic_message_severity::error, condition_origin, explanation);
	}

	// action-specific (none)

	// autogen-specific

	void push_error_autogen_incompatible_condition(
		lang::position_tag autogen_origin,
		lang::position_tag condition_origin,
		std::string_view required_matching_value);

	void push_error_autogen_forbidden_condition(
		lang::position_tag autogen_origin,
		lang::position_tag condition_origin);

	void push_error_autogen_missing_condition(
		lang::position_tag visibility_origin,
		lang::position_tag autogen_origin,
		std::string_view missing_condition);

	// other

	void push_error_internal_compiler_error(std::string_view function_name, lang::position_tag origin);

private:
	void push_error_bound_redefinition(
		bool is_lower,
		lang::position_tag redefinition,
		lang::position_tag original);

	void push_message_dead_condition(
		diagnostic_message_severity severity,
		lang::position_tag condition_origin,
		std::string_view explanation)
	{
		push_message(make_diagnostic_message(
			severity, diagnostic_message_id::dead_condition, condition_origin, "dead condition: ", explanation));
	}

	container_type m_messages;
	std::vector<lang::position_tag> m_layers;
};

} // namespace fs::compiler
