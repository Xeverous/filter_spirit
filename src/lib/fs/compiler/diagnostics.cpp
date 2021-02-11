#include <fs/compiler/diagnostics.hpp>
#include <fs/parser/parser.hpp>
#include <fs/log/logger.hpp>

namespace fs::compiler
{

using dmid = diagnostic_message_id;

void push_error_invalid_integer_value(
	int min,
	int max,
	lang::integer actual,
	diagnostics_container& diagnostics)
{
	diagnostics.push_back(make_error(
		dmid::invalid_integer_value,
		actual.origin,
		"invalid integer value, expected value in range ",
		std::to_string(min),
		" - ",
		std::to_string(max),
		" but got ",
		std::to_string(actual.value)));
}

void push_error_invalid_amount_of_arguments(
	int expected_min,
	boost::optional<int> expected_max,
	int actual,
	lang::position_tag origin,
	diagnostics_container& diagnostics)
{
	if (expected_max) {
		diagnostics.push_back(make_error(
			dmid::invalid_amount_of_arguments,
			origin,
			"invalid amount of arguments, expected from ",
			std::to_string(expected_min),
			" to ",
			std::to_string(*expected_max),
			" but got ",
			std::to_string(actual)));
	}
	else {
		diagnostics.push_back(make_error(
			dmid::invalid_amount_of_arguments,
			origin,
			"invalid amount of arguments, expected at least ",
			std::to_string(expected_min),
			" but got ",
			std::to_string(actual)));
	}
}

void detail::push_error_bound_redefinition(
	bool is_lower,
	lang::position_tag redefinition,
	lang::position_tag original,
	diagnostics_container& diagnostics)
{
	const auto id = is_lower ? dmid::lower_bound_redefinition : dmid::upper_bound_redefinition;
	const auto str = is_lower ? "lower" : "upper";
	diagnostics.push_back(make_error(
		id,
		redefinition,
		str,
		" bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)"));
	diagnostics.push_back(make_note_first_defined_here(original));
}

void push_error_type_mismatch(
	lang::object_type expected,
	lang::object_type actual,
	lang::position_tag origin,
	diagnostics_container& diagnostics)
{
	diagnostics.push_back(make_error(
		dmid::type_mismatch,
		origin,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string_view(expected),
		"' but got '",
		fs::lang::to_string_view(actual),
		"'"));
}

void push_error_internal_compiler_error(
	std::string_view function_name,
	lang::position_tag origin,
	diagnostics_container& diagnostics)
{
	diagnostics.push_back(make_error(
		dmid::internal_compiler_error,
		origin,
		log::strings::internal_compiler_error,
		"caused by: ",
		function_name,
		"\n",
		log::strings::request_bug_report));
}

void push_error_autogen_error(
	autogen_error error,
	diagnostics_container& diagnostics)
{
	const auto error_str = [&]() {
		switch (error.cause) {
			using cause_t = autogen_error_cause;

			case cause_t::expected_empty_condition:
				return "expected this condition to be empty";
			case cause_t::invalid_rarity_condition:
				return "expected no rarity condition or one that allows 'Unique'";
			case cause_t::invalid_class_condition:
				return "invalid class condition"; // TODO this is underspecified
			// intentionally no default case
			// GCC warns when a switch has no default and does not cover all enums
		}

		return "(unknown)";
	}();

	diagnostics.push_back(make_error(dmid::autogen_error, error.visibility_origin, "autogeneration here with invalid combination of conditions"));
	diagnostics.push_back(make_note_minor(error.condition_origin, error_str));
	diagnostics.push_back(make_note_minor(error.autogen_origin, "autogeneration specified here"));
}

void output_diagnostics(
	const diagnostics_container& messages,
	const parser::parse_metadata& metadata,
	log::logger& logger)
{
	for (const diagnostic_message& msg : messages) {
		const log::severity s =
			msg.severity == diagnostic_message_severity::error   ? log::severity::error   :
			msg.severity == diagnostic_message_severity::warning ? log::severity::warning :
			log::severity::info;

		auto stream = logger.message(s);

		if (msg.origin) {
			// TODO optimize print code to use parser::line_lookup
			stream.print_line_number_with_description_and_underlined_code(
				metadata.lookup.get_view_of_whole_content(),
				metadata.lookup.position_of(*msg.origin),
				msg.description);
		}
		else {
			stream << msg.description;
		}

		stream << "\n";
	}
}

}
