#include "print/compile_error.hpp"
#include "print/generic.hpp"
#include "compiler/error.hpp"

namespace
{

namespace fp = fs::print;
using iterator_type = fp::iterator_type;
using range_type = fp::range_type;

void print_compile_error(
	fs::compiler::error::name_already_exists error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.duplicated_name,
		fp::compiler_error_string,
		"name already exists");
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.original_name,
		fp::note_string,
		"first defined here");
}

void print_compile_error(
	fs::compiler::error::no_such_name error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.name_origin,
		fp::compiler_error_string,
		"no such name exists");
}

void print_compile_error(
	fs::compiler::error::type_mismatch_in_assignment error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.right_operand_value_origin,
		fp::compiler_error_string,
		"type mismatch in assignment, operands of types '",
		fs::lang::to_string(error.left_operand_type),
		"' and '",
		fs::lang::to_string(error.right_operand_type),
		"'");

	if (error.right_operand_type_origin != error.right_operand_value_origin)
		fp::print_line_number_with_indication_and_texts(
			error_stream,
			content_range,
			error.right_operand_type_origin,
			fp::note_string,
			"right operand type defined here");
}

void print_compile_error(
	fs::compiler::error::type_mismatch_in_expression error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.expression_type_origin,
		fp::compiler_error_string,
		"type mismatch in expression, expected expression of type '",
		fs::lang::to_string(error.expected_type),
		"' but got '",
		fs::lang::to_string(error.actual_type),
		"'");
}

void print_compile_error(
	fs::compiler::error::non_homogeneous_array error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.first_expr_origin,
		fp::compiler_error_string,
		"non homogeneous array, one operand of type '",
		fs::lang::to_string(error.first_expr_type),
		"'");

	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.second_expr_origin,
		fp::note_string,
		"and one operand of type '",
		fs::lang::to_string(error.second_expr_type),
		"'");
}

void print_compile_error(
	fs::compiler::error::single_object_to_array_assignment error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.right_operand_expr,
		fp::compiler_error_string,
		"type mismatch in expression, operands of types '",
		fs::lang::to_string(error.left_operand_type),
		"' and '",
		fs::lang::to_string(error.right_operand_type),
		"'\nperhaps you meant to write '[expression]' instead of 'expression'");
}

void print_compile_error(
	fs::compiler::error::nested_arrays_not_allowed error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.expression_origin,
		fp::compiler_error_string,
		"nested arrays are not allowed");
}

void print_compile_error(
	fs::compiler::error::internal_error_while_parsing_constant error,
	range_type content_range,
	std::ostream& error_stream)
{
	error_stream << "INTERNAL COMPILER ERROR while parsing a type definition expression\n"
		"please report a bug with attached minimal source that reproduces it\n";

	constexpr auto note = "while parsing this";

	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.wanted_type_origin,
		fp::note_string,
		note);
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.wanted_name_origin,
		fp::note_string,
		note);
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.expression_type_origin,
		fp::note_string,
		note);

	if (error.expression_value_origin != error.expression_type_origin)
		fp::print_line_number_with_indication_and_texts(
			error_stream,
			content_range,
			error.expression_value_origin,
			fp::note_string,
			note);

	if (error.expression_name_origin)
		fp::print_line_number_with_indication_and_texts(
			error_stream,
			content_range,
			*error.expression_name_origin,
			fp::note_string,
			note);
}

void print_compile_error(
	fs::compiler::error::duplicate_action error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.second_action_origin,
		fp::compiler_error_string,
		"action duplication");

	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.first_action_origin,
		fp::note_string,
		"first defined here");
}

void print_compile_error(
	fs::compiler::error::duplicate_condition error,
	range_type content_range,
	std::ostream& error_stream)
{
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.second_condition_origin,
		fp::compiler_error_string,
		"condition duplication");

	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.first_condition_origin,
		fp::note_string,
		"first defined here");
}

}

namespace fs::print
{

void compile_error(
	compiler::error::error_variant error,
	parser::range_type content_range,
	std::ostream& error_stream)
{
	std::visit(
		[&](auto error) { print_compile_error(error, content_range, error_stream); },
		error);
}

}
