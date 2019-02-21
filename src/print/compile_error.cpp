#include "print/compile_error.hpp"
#include "print/generic.hpp"
#include <string>

namespace
{

using namespace fs;

void print_compile_error(
	compiler::error::name_already_exists error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_duplicated_name),
		print::compiler_error_string,
		"name already exists");
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_original_name),
		print::note_string,
		"first defined here");
}

void print_compile_error(
	compiler::error::no_such_name error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		print::compiler_error_string,
		"no such name exists");
}

void print_compile_error(
	compiler::error::no_such_function error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_name),
		print::compiler_error_string,
		"no such function exists");
}

void print_compile_error(
	compiler::error::invalid_amount_of_arguments error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	std::string expected_arguments_string = [&error]()
	{
		if (error.min_expected == error.max_expected)
			return std::to_string(error.min_expected);
		else
			return std::to_string(error.min_expected) + " - " + std::to_string(error.max_expected);
	}();

	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_arguments),
		print::compiler_error_string,
		"invalid amount of arguments, expected ",
		expected_arguments_string,
		" but got ",
		error.actual);
}

void print_compile_error(
	compiler::error::type_mismatch error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_expression),
		print::compiler_error_string,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string(error.expected_type),
		"' but got '",
		fs::lang::to_string(error.actual_type),
		"'");
}

void print_compile_error(
	compiler::error::nested_arrays_not_allowed error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_nested_array_expression),
		print::compiler_error_string,
		"nested arrays are not allowed");
}

void print_compile_error(
	compiler::error::non_homogeneous_array error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_first_element),
		print::compiler_error_string,
		"non homogeneous array, one operand of type '",
		fs::lang::to_string(error.first_element_type),
		"'");

	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_second_element),
		print::note_string,
		"and one operand of type '",
		fs::lang::to_string(error.second_element_type),
		"'");
}

void print_compile_error(
	compiler::error::empty_socket_group error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		print::compiler_error_string,
		"socket group can not be empty");
}

void print_compile_error(
	compiler::error::illegal_characters_in_socket_group error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		print::compiler_error_string,
		"invalid socket group (use only R/G/B/W characters)");
}

void print_compile_error(
	compiler::error::invalid_socket_group error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_socket_group_string),
		print::compiler_error_string,
		"invalid socket group");
}

void print_compile_error(
	compiler::error::invalid_minimap_icon_size error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_size_argument),
		print::compiler_error_string,
		"invalid minimap icon size, valid values are 0 - 2, requested is ",
		error.requested_size);
}

void print_compile_error(
	compiler::error::positional_sound_not_supported error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		print::compiler_error_string,
		"sorry, positional sound is not [yet] supported");
}

void print_compile_error(
	compiler::error::disable_drop_sound_not_supported error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		print::compiler_error_string,
		"sorry, disabling default drop sound is not [yet] supported");
}

void print_compile_error(
	compiler::error::temporary_beam_effect_not_supported error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_2nd_argument),
		print::compiler_error_string,
		"sorry, temporary beam effect is not [yet] supported");
}

void print_compile_error(
	compiler::error::condition_redefinition error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		print::compiler_error_string,
		"condition redefinition (the same condition can not be specified again in the same block or nested blocks)");
}

void print_compile_error(
	compiler::error::lower_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		print::compiler_error_string,
		"lower bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
}

void print_compile_error(
	compiler::error::upper_bound_redefinition error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		print::compiler_error_string,
		"upper bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)");
}

void print_compile_error(
	compiler::error::exact_comparison_redefinition error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		print::compiler_error_string,
		"exact comparison redefinition (exact comparison can not be specified again in the same block or nested blocks)");
}

void print_compile_error(
	compiler::error::exact_comparison_outside_parent_range error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_redefinition),
		print::compiler_error_string,
		"exact comparison outside parent range (no item can ever satisfy this requirement because specified value does not fit into parent block range)");
}

void print_compile_error(
	compiler::error::internal_compiler_error_during_action_evaluation error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_action),
		print::internal_compiler_error_string,
		"unhandled case in action evaluation\n",
		print::internal_compiler_error_info);
}

void print_compile_error(
	compiler::error::internal_compiler_error_during_range_evaluation error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_comparison_condition),
		print::internal_compiler_error_string,
		"unhandled case in range evaluation\n",
		print::internal_compiler_error_info);
}

void print_compile_error(
	compiler::error::internal_compiler_error_during_comparison_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_comparison_condition),
		print::internal_compiler_error_string,
		"unhandled case in comparison condition evaluation\n",
		print::internal_compiler_error_info);
}

void print_compile_error(
	compiler::error::internal_compiler_error_during_string_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_string_condition),
		print::internal_compiler_error_string,
		"unhandled case in string condition evaluation\n",
		print::internal_compiler_error_info);
}

void print_compile_error(
	compiler::error::internal_compiler_error_during_boolean_condition_evaluation error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		lookup_data.position_of(error.place_of_boolean_condition),
		print::internal_compiler_error_string,
		"unhandled case in boolean condition evaluation\n",
		print::internal_compiler_error_info);
}

}

namespace fs::print
{

void compile_error(
	compiler::error::error_variant error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	std::visit(
		[&](auto error) { print_compile_error(error, lookup_data, error_stream); },
		error);
}

}
