#include <fs/compiler/print_error.hpp>
#include <fs/parser/parser.hpp>
#include <fs/log/logger.hpp>
#include <fs/log/strings.hpp>
#include <fs/utility/algorithm.hpp>

#include <cassert>

namespace
{

using namespace fs;
using namespace fs::compiler;

// recursion point for errors that contain other errors
void print_error_variant(
	const compile_error& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger);

void print_error_impl(
	errors::name_already_exists error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_duplicated_name),
		log::strings::error,
		"name already exists");
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_name),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::no_such_name error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such name exists");
}

void print_error_impl(
	errors::no_such_function error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such function exists");
}

void print_error_impl(
	errors::no_such_query error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such query exists");
}

void print_error_impl(
	errors::invalid_amount_of_arguments error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
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
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_expression),
		log::strings::error,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string_view(error.expected_type),
		"' but got '",
		fs::lang::to_string_view(error.actual_type),
		"'");
}

void print_unmatched_function_call(
	std::string_view function_name,
	const errors::unmatched_function_call& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger << "candidate " << function_name << "(";
	utility::for_each_and_between(
		error.expected_argument_types.begin(),
		error.expected_argument_types.end(),
		[&logger](fs::lang::object_type type) { logger << fs::lang::to_string_view(type); },
		[&logger]() { logger << ", "; });
	logger << "):\n";

	print_error_variant(error.error, lookup_data, logger);
}

void print_error_impl(
	const errors::failed_constructor_call& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	const std::string_view attempted_type_name = fs::lang::to_string_view(error.attempted_type_to_construct);

	const std::string_view all_code = lookup_data.get_view_of_whole_content();
	const std::string_view code_to_underline = lookup_data.position_of(error.place_of_function_call);

	logger.print_line_number(log::count_lines(all_code.data(), code_to_underline.data()));
	logger << log::strings::error << "failed constructor call " << attempted_type_name << "(";
	utility::for_each_and_between(
		error.ctor_argument_types.begin(),
		error.ctor_argument_types.end(),
		[&logger](fs::lang::object_type type) { logger << fs::lang::to_string_view(type); },
		[&logger]() { logger << ", "; });
	logger << "):\n";
	logger.print_underlined_code(all_code, code_to_underline);

	assert(error.error != nullptr);
	print_error_variant(*error.error, lookup_data, logger);
}

void print_error_impl(
	const errors::no_matching_constructor_found& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	const std::string_view attempted_type_name = fs::lang::to_string_view(error.attempted_type_to_construct);

	const std::string_view all_code = lookup_data.get_view_of_whole_content();
	const std::string_view code_to_underline = lookup_data.position_of(error.place_of_function_call);

	logger.print_line_number(log::count_lines(all_code.data(), code_to_underline.data()));
	logger << log::strings::error << "no matching constructor for call to " << attempted_type_name << "(";
	utility::for_each_and_between(
		error.supplied_types.begin(),
		error.supplied_types.end(),
		[&logger](std::optional<fs::lang::object_type> type)
		{
			if (type.has_value())
				logger << fs::lang::to_string_view(*type);
			else
				logger << "?";
		},
		[&logger]() { logger << ", "; });
	logger << "):\n";

	logger.print_underlined_code(all_code, code_to_underline);

	for (const errors::unmatched_function_call& inner_error : error.errors)
		print_unmatched_function_call(attempted_type_name, inner_error, lookup_data, logger);
}

void print_error_impl(
	errors::nested_arrays_not_allowed error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_nested_array_expression),
		log::strings::error,
		"nested arrays are not allowed");
}

void print_error_impl(
	errors::non_homogeneous_array error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_first_element),
		log::strings::error,
		"non homogeneous array, one element of type '",
		fs::lang::to_string_view(error.first_element_type),
		"'");
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_second_element),
		log::strings::note,
		"and one element of type '",
		fs::lang::to_string_view(error.second_element_type),
		"'");
}

void print_error_impl(
	errors::index_out_of_range error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_subscript),
		log::strings::error,
		"index out of range, requested ",
		error.requested_index,
		" but array size is ",
		error.array_size);
}

void print_error_impl(
	errors::empty_socket_group error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"socket group can not be empty");
}

void print_error_impl(
	errors::illegal_characters_in_socket_group error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"invalid socket group (use only R/G/B/W characters)");
}

void print_error_impl(
	errors::invalid_socket_group error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"invalid socket group");
}

void print_error_impl(
	errors::invalid_minimap_icon_size error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_size_argument),
		log::strings::error,
		"invalid minimap icon size, valid values are 0 - 2, requested is ",
		error.requested_size);
}

void print_error_impl(
	errors::condition_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"condition redefinition (the same condition can not be specified again in the same block or nested blocks)");
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::lower_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"lower bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::upper_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"upper bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_definition),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	errors::internal_compiler_error_during_action_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		log::strings::internal_compiler_error,
		"unhandled case in action evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	errors::internal_compiler_error_during_range_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_comparison_condition),
		log::strings::internal_compiler_error,
		"unhandled case in range evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	errors::internal_compiler_error_during_comparison_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_comparison_condition),
		log::strings::internal_compiler_error,
		"unhandled case in comparison condition evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	errors::internal_compiler_error_during_string_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_string_condition),
		log::strings::internal_compiler_error,
		"unhandled case in string condition evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	errors::internal_compiler_error_during_boolean_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.print_line_number_with_description_and_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_boolean_condition),
		log::strings::internal_compiler_error,
		"unhandled case in boolean condition evaluation\n",
		log::strings::request_bug_report);
}

void print_error_variant(
	const compile_error& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	std::visit(
		[&](const auto& error) { print_error_impl(error, lookup_data, logger); },
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
	logger.begin_error_message();
	logger << "compile error\n";
	// some errors are recursive (contain other errors)
	// print_error_variant() is a helper for recursion
	// if we recursed print_error() we would get multiple log messages
	print_error_variant(error, lookup_data, logger);
	logger.end_message();
}

}
