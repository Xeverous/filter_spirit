#include "fs/compiler/print_error.hpp"
#include "fs/log/logger.hpp"
#include "fs/log/strings.hpp"

#include <string>

namespace
{

using namespace fs;

void print_error_impl(
	compiler::error::name_already_exists error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_duplicated_name),
		log::strings::error,
		"name already exists");
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_original_name),
		log::strings::note,
		"first defined here");
}

void print_error_impl(
	compiler::error::no_such_name error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such name exists");
}

void print_error_impl(
	compiler::error::no_such_function error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such function exists");
}

void print_error_impl(
	compiler::error::no_such_query error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		log::strings::error,
		"no such query exists");
}

void print_error_impl(
	compiler::error::invalid_amount_of_arguments error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	const std::string expected_arguments_string = [&error]()
	{
		if (error.min_expected == error.max_expected)
			return std::to_string(error.min_expected);
		else
			return std::to_string(error.min_expected) + " - " + std::to_string(error.max_expected);
	}();

	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_arguments),
		log::strings::error,
		"invalid amount of arguments, expected ",
		expected_arguments_string,
		" but got ",
		error.actual);
}

void print_error_impl(
	compiler::error::type_mismatch error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
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
	compiler::error::nested_arrays_not_allowed error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_nested_array_expression),
		log::strings::error,
		"nested arrays are not allowed");
}

void print_error_impl(
	compiler::error::non_homogeneous_array error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_first_element),
		log::strings::error,
		"non homogeneous array, one operand of type '",
		fs::lang::to_string_view(error.first_element_type),
		"'");
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_second_element),
		log::strings::note,
		"and one operand of type '",
		fs::lang::to_string_view(error.second_element_type),
		"'");
}

void print_error_impl(
	compiler::error::empty_socket_group error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"socket group can not be empty");
}

void print_error_impl(
	compiler::error::illegal_characters_in_socket_group error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"invalid socket group (use only R/G/B/W characters)");
}

void print_error_impl(
	compiler::error::invalid_socket_group error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		log::strings::error,
		"invalid socket group");
}

void print_error_impl(
	compiler::error::invalid_minimap_icon_size error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_size_argument),
		log::strings::error,
		"invalid minimap icon size, valid values are 0 - 2, requested is ",
		error.requested_size);
}

void print_error_impl(
	compiler::error::positional_sound_not_supported error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		log::strings::error,
		"sorry, positional sound is not [yet] supported");
}

void print_error_impl(
	compiler::error::disable_drop_sound_not_supported error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		log::strings::error,
		"sorry, disabling default drop sound is not [yet] supported");
}

void print_error_impl(
	compiler::error::temporary_beam_effect_not_supported error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_2nd_argument),
		log::strings::error,
		"sorry, temporary beam effect is not [yet] supported");
}

void print_error_impl(
	compiler::error::condition_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"condition redefinition (the same condition can not be specified again in the same block or nested blocks)");
}

void print_error_impl(
	compiler::error::lower_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"lower bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
}

void print_error_impl(
	compiler::error::upper_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"upper bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
}

void print_error_impl(
	compiler::error::exact_comparison_redefinition error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"exact comparison redefinition (exact comparison can not be specified again in the same block or nested blocks)");
}

void print_error_impl(
	compiler::error::exact_comparison_outside_parent_range error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		log::strings::error,
		"exact comparison outside parent range (no item can ever satisfy this requirement because specified value does not fit into parent block range)");
}

void print_error_impl(
	compiler::error::internal_compiler_error_during_action_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		log::strings::internal_compiler_error,
		"unhandled case in action evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	compiler::error::internal_compiler_error_during_range_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_comparison_condition),
		log::strings::internal_compiler_error,
		"unhandled case in range evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	compiler::error::internal_compiler_error_during_comparison_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_comparison_condition),
		log::strings::internal_compiler_error,
		"unhandled case in comparison condition evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	compiler::error::internal_compiler_error_during_string_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_string_condition),
		log::strings::internal_compiler_error,
		"unhandled case in string condition evaluation\n",
		log::strings::request_bug_report);
}

void print_error_impl(
	compiler::error::internal_compiler_error_during_boolean_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		lookup_data.position_of(error.place_of_boolean_condition),
		log::strings::internal_compiler_error,
		"unhandled case in boolean condition evaluation\n",
		log::strings::request_bug_report);
}

}

namespace fs::compiler
{

void print_error(
	error::error_variant error,
	const parser::lookup_data& lookup_data,
	log::logger& logger)
{
	logger.begin_error_message();
	logger << "compile error\n";
	std::visit(
		[&](auto error) { print_error_impl(error, lookup_data, logger); },
		error);
	logger.end_message();
}

}
