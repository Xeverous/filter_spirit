#include <fs/lang/limits.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/outcome.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/visitor.hpp>
#include <fs/utility/algorithm.hpp>
#include <fs/utility/string_helpers.hpp>

namespace
{

using namespace fs;
using namespace fs::compiler;

// ---- errors ----

void output_error_impl(
	errors::name_already_exists error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.duplicated_name),
		log::strings::error,
		"name already exists");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.original_name),
		log::strings::note,
		"first defined here");
}

void output_error_impl(
	errors::no_such_name error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.name),
		log::strings::error,
		"no such name exists");
}

void output_error_impl(
	errors::invalid_amount_of_arguments error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	if (error.max_allowed) {
		stream.print_line_number_with_description_and_underlined_code(
			lookup_data.get_view_of_whole_content(),
			lookup_data.position_of(error.arguments),
			log::strings::error,
			"invalid amount of arguments, expected from ",
			error.min_allowed,
			" to ",
			*error.max_allowed,
			" but got ",
			error.actual);
	}
	else {
		stream.print_line_number_with_description_and_underlined_code(
			lookup_data.get_view_of_whole_content(),
			lookup_data.position_of(error.arguments),
			log::strings::error,
			"invalid amount of arguments, expected ",
			error.min_allowed,
			" but got ",
			error.actual);
	}
}

void output_error_impl(
	errors::invalid_integer_value error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	if (error.max_allowed_value) {
		stream.print_line_number_with_description_and_underlined_code(
			lookup_data.get_view_of_whole_content(),
			lookup_data.position_of(error.origin),
			log::strings::error,
			"invalid integer value, expected value in range ",
			error.min_allowed_value,
			" - ",
			*error.max_allowed_value,
			" but got ",
			error.actual_value);
	}
	else {
		stream.print_line_number_with_description_and_underlined_code(
			lookup_data.get_view_of_whole_content(),
			lookup_data.position_of(error.origin),
			log::strings::error,
			"invalid inetger value, expected at least ",
			error.min_allowed_value,
			" but got ",
			error.actual_value);
	}
}

void output_error_impl(
	errors::type_mismatch error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.expression),
		log::strings::error,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string_view(error.expected_type),
		"' but got '",
		fs::lang::to_string_view(error.actual_type),
		"'");
}

void output_error_impl(
	errors::illegal_character_in_socket_spec error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.origin),
		log::strings::error,
		"illegal character in socket group at position ",
		error.character_index,
		" (use only R/G/B/W/A/D characters)");
}

void output_error_impl(
	errors::invalid_socket_spec error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.origin),
		log::strings::error,
		"invalid socket group");
}

void output_error_impl(
	errors::duplicate_influence error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.second_influence),
		log::strings::error,
		"duplicate influence");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.first_influence),
		log::strings::note,
		"first defined here");
}

void output_error_impl(
	errors::condition_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.redefinition),
		log::strings::error,
		"condition redefinition (the same condition can not be specified again in the same block or nested blocks)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.original_definition),
		log::strings::note,
		"first defined here");
}

void output_error_impl(
	errors::action_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.redefinition),
		log::strings::error,
		"action redefinition (the same action can not be specified again in the same block)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.original_definition),
		log::strings::note,
		"first defined here");
}

void output_error_impl(
	errors::lower_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.redefinition),
		log::strings::error,
		"lower bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.original_definition),
		log::strings::note,
		"first defined here");
}

void output_error_impl(
	errors::upper_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.redefinition),
		log::strings::error,
		"upper bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.original_definition),
		log::strings::note,
		"first defined here");
}

void output_error_impl(
	errors::invalid_builtin_alert_sound_id error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.origin),
		log::strings::error,
		"invalid built-in alert sound ID: should be integer ID or Shaper voice line");
}

void output_error_impl(
	errors::invalid_set_alert_sound error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.sequence_origin),
		log::strings::error,
		"invalid alert sound: does not match any of: built-in (1-2 integers), custom (1 string)");
}

void output_error_impl(
	errors::price_without_autogen error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.visibility_origin),
		log::strings::error,
		"generation of a block with price bound has missing autogeneration specifier");

	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.price_origin),
		log::strings::note,
		"price bound specified here");

	if (error.another_price_origin) {
		stream.print_line_number_with_description_and_underlined_code(
			lookup_data.get_view_of_whole_content(),
			lookup_data.position_of(*error.another_price_origin),
			log::strings::note,
			"another price bound specified here");
	}
}

void output_error_impl(
	errors::autogen_error error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	const auto error_str = [&]() {
		switch (error.cause) {
			using cause_t = errors::autogen_error_cause;

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

	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.visibility_origin),
		log::strings::error,
		"autogeneration here with invalid combination of conditions");

	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.condition_origin),
		log::strings::note,
		error_str);

	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.autogen_origin),
		log::strings::note,
		"autogeneration specified here");
}

void output_error_impl(
	errors::internal_compiler_error error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.origin),
		log::strings::internal_compiler_error,
		error.cause._to_string(),
		"\n",
		log::strings::request_bug_report);
}

void output_error_variant(
	const error& err,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	std::visit(
		[&](const auto& err) { output_error_impl(err, lookup_data, stream); },
		err);
}

// ---- warnings ----

void output_warning_impl(
	warnings::font_size_outside_range warn,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(warn.origin),
		log::strings::warning,
		"font size outside allowed range (",
		lang::limits::min_filter_font_size,
		" - ",
		lang::limits::max_filter_font_size,
		")");
}

void output_warning_variant(
	const warning& warn,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	std::visit(
		[&](const auto& warn) { output_warning_impl(warn, lookup_data, stream); },
		warn);
}

// ---- notes ----

void output_note_impl(
	notes::fixed_text note,
	const parser::lookup_data& /* lookup_data */,
	log::message_stream& stream)
{
	stream << note.text;
}

void output_note_impl(
	notes::failed_operations_count note,
	const parser::lookup_data& /* lookup_data */,
	log::message_stream& stream)
{
	stream << "failed " << note.operation_name << ": " << note.count;
}

void output_note_variant(
	const note& n,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	std::visit(
		[&](const auto& n) { output_note_impl(n, lookup_data, stream); },
		n);
}

// ---- main dispatching ----


// For case when there are/will be multiple errors:
// we call a separate function to hold 1 stream message

void output_error(
	const error& err,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	auto stream = logger.error();
	stream << "error:\n";
	output_error_variant(err, lookup_data, stream);
}

void output_warning(
	const warning& warn,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	auto stream = logger.warning();
	stream << "warning:\n";
	output_warning_variant(warn, lookup_data, stream);
}

void output_note(
	const note& n,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	auto stream = logger.info();
	stream << "note:\n";
	output_note_variant(n, lookup_data, stream);
}

}

namespace fs::compiler
{

void output_logs(
	const log_container& logs,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	for (const auto& msg : logs) {
		std::visit(utility::visitor{
			[&](const error& err) { output_error(err, lookup_data, logger); },
			[&](const warning& warn) { output_warning(warn, lookup_data, logger); },
			[&](const note& n) { output_note(n, lookup_data, logger); }
		}, msg);
	}
}

}
