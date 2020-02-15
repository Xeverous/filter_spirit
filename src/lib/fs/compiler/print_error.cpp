#include <fs/compiler/print_error.hpp>
#include <fs/parser/parser.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/algorithm.hpp>
#include <fs/utility/string_helpers.hpp>

#include <cassert>

namespace
{

using namespace fs;
using namespace fs::compiler;

// recursion point for errors that contain other errors
void print_error_variant(
	const compile_error& error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream);

void print_error_impl(
	errors::name_already_exists error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_duplicated_name),
		log::strings::error,
		"name already exists");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_name),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::no_such_name error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such name exists");
}

void print_error_impl(
	errors::no_such_query error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such query exists");
}

void print_error_impl(
	errors::invalid_amount_of_arguments error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_arguments),
		log::strings::error,
		"invalid amount of arguments, expected ",
		error.expected,
		" but got ",
		error.actual);
}

void print_error_impl(
	errors::type_mismatch error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_expression),
		log::strings::error,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string_view(error.expected_type),
		"' but got '",
		fs::lang::to_string_view(error.actual_type),
		"'");
}

void print_error_impl(
	errors::empty_socket_group error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"socket group can not be empty");
}

void print_error_impl(
	errors::illegal_characters_in_socket_group error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"invalid socket group (use only R/G/B/W characters)");
}

void print_error_impl(
	errors::invalid_socket_group error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"invalid socket group");
}

void print_error_impl(
	errors::invalid_minimap_icon_size error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_size_argument),
		log::strings::error,
		"invalid minimap icon size, valid values are 0 - 2, requested is ",
		error.requested_size);
}

void print_error_impl(
	errors::condition_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"condition redefinition (the same condition can not be specified again in the same block or nested blocks)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::action_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"action redefinition (the same action can not be specified again in the same block)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::lower_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"lower bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::upper_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"upper bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::internal_compiler_error error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	const auto error_str = [&]() {
		switch (error.cause) {
			using error_cause = errors::internal_compiler_error_cause;

			case error_cause::add_boolean_condition:
				return "add_boolean_condition";
			case error_cause::add_range_condition:
				return "add_range_condition";
			case error_cause::real_filter_add_numeric_condition:
				return "real_filter_add_numeric_condition";
			case error_cause::real_filter_add_string_array_condition:
				return "real_filter_add_string_array_condition";
			case error_cause::real_filter_add_color_action:
				return "real_filter_add_color_action";
			case error_cause::spirit_filter_add_numeric_comparison_condition:
				return "spirit_filter_add_numeric_comparison_condition";
			case error_cause::spirit_filter_add_unary_action:
				return "spirit_filter_add_unary_action";
			default:
				return "(unknown)";
		}
	}();

	stream.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.origin),
		log::strings::internal_compiler_error,
		error_str,
		"\n",
		log::strings::request_bug_report);
}

void print_error_variant(
	const compile_error& error,
	const parser::lookup_data& lookup_data,
	log::message_stream& stream)
{
	std::visit(
		[&](const auto& error) { print_error_impl(error, lookup_data, stream); },
		error);
}

}

namespace fs::compiler
{

void print_error(
	const compile_error& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	auto stream = logger.error();
	stream << "compile error\n";
	// some errors are recursive (contain other errors)
	// print_error_variant() is a helper for recursion
	// if we recursed print_error() we would get multiple log messages
	print_error_variant(error, lookup_data, stream);
}

}
