#include <fs/compiler/diagnostics.hpp>
#include <fs/parser/parser.hpp>
#include <fs/log/logger.hpp>

namespace fs::compiler
{

using dmid = diagnostic_message_id;

void diagnostics_store::push_error_value_out_of_range(int min, int max, lang::integer actual)
{
	push_message(make_error(
		dmid::value_out_of_range,
		actual.origin,
		"invalid integer value, expected value in range ",
		std::to_string(min),
		" - ",
		std::to_string(max),
		" but got ",
		std::to_string(actual.value)));
}

void diagnostics_store::push_error_invalid_amount_of_arguments(
	int expected_min,
	boost::optional<int> expected_max,
	int actual,
	lang::position_tag origin)
{
	if (expected_max) {
		push_message(make_error(
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
		push_message(make_error(
			dmid::invalid_amount_of_arguments,
			origin,
			"invalid amount of arguments, expected at least ",
			std::to_string(expected_min),
			" but got ",
			std::to_string(actual)));
	}
}

void diagnostics_store::push_error_bound_redefinition(bool is_lower, lang::position_tag redefinition, lang::position_tag original)
{
	const auto id = is_lower ? dmid::lower_bound_redefinition : dmid::upper_bound_redefinition;
	const auto str = is_lower ? "lower" : "upper";
	push_message(make_error(
		id,
		redefinition,
		str,
		" bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)"));
	push_message(make_note_first_defined_here(original));
}

void diagnostics_store::push_error_type_mismatch(lang::object_type expected, lang::object_type actual, lang::position_tag origin)
{
	push_message(make_error(
		dmid::type_mismatch,
		origin,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string_view(expected),
		"' but got '",
		fs::lang::to_string_view(actual),
		"'"));
}

void diagnostics_store::push_error_internal_compiler_error(std::string_view function_name, lang::position_tag origin)
{
	push_message(make_error(
		dmid::internal_compiler_error,
		origin,
		log::strings::internal_compiler_error,
		"caused by: ",
		function_name,
		"\n",
		log::strings::request_bug_report));
}

void diagnostics_store::push_error_autogen_incompatible_condition(
	lang::position_tag autogen_origin,
	lang::position_tag condition_origin,
	std::string_view required_matching_value)
{
	push_message(make_error(
		dmid::autogen_incompatible_condition,
		condition_origin,
		"autogen-incompatible condition: it should either not exist or allow value ",
		required_matching_value));
	push_message(make_note_minor(autogen_origin, "autogeneration specified here"));
}

void diagnostics_store::push_error_autogen_forbidden_condition(
	lang::position_tag autogen_origin,
	lang::position_tag condition_origin)
{
	push_message(make_error(
		dmid::autogen_forbidden_condition,
		condition_origin,
		"autogen-forbidden condition: it should not be present for this autogen"));
	push_message(make_note_minor(autogen_origin, "autogeneration specified here"));
}

void diagnostics_store::push_error_autogen_missing_condition(
	lang::position_tag visibility_origin,
	lang::position_tag autogen_origin,
	std::string_view required_condition)
{
	push_message(make_error(
		dmid::autogen_missing_condition,
		visibility_origin,
		"autogen with missing condition: ",
		required_condition));
	push_message(make_note_minor(autogen_origin, "autogeneration specified here"));
}

void diagnostics_store::output_messages(const parser::parse_metadata& metadata, log::logger& logger) const
{
	for (const diagnostic_message& msg : m_messages) {
		const log::severity s =
			msg.severity == diagnostic_message_severity::error   ? log::severity::error   :
			msg.severity == diagnostic_message_severity::warning ? log::severity::warning :
			log::severity::info;

		auto stream = logger.message(s);

		if (msg.origin) {
			parser::underlined_text ut = metadata.get_underlined_text(*msg.origin);
			stream.print_line_number_with_description(static_cast<int>(ut.context.line_number), msg.description);
			stream.print_underlined_code(ut.context.surrounding_lines, ut.underline);
		}
		else {
			stream << msg.description;
		}

		stream << "\n";
	}
}

}
