#include "print/compile_error.hpp"
#include "print/generic.hpp"
#include "compiler/error.hpp"

namespace
{

namespace fp = fs::print;
using iterator_type = fp::iterator_type;
using range_type = fp::range_type;

void print_compile_error(
	const fs::parser::lookup_data& lookup_data,
	fs::compiler::error::name_already_exists error,
	std::ostream& error_stream)
{
	const range_type content_range = lookup_data.get_range_of_whole_content();
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
	const fs::parser::lookup_data& lookup_data,
	fs::compiler::error::no_such_name error,
	std::ostream& error_stream)
{
	const range_type content_range = lookup_data.get_range_of_whole_content();
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.name_origin,
		fp::compiler_error_string,
		"no such name exists");
}

void print_compile_error(
	const fs::parser::lookup_data& lookup_data,
	fs::compiler::error::type_mismatch error,
	std::ostream& error_stream)
{
	const range_type content_range = lookup_data.get_range_of_whole_content();
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.right_operand_value_origin,
		fp::compiler_error_string,
		"type mismatch in expression, operands of types '",
		fs::lang::to_string(error.left_operand_type),
		"' and '",
		fs::lang::to_string(error.right_operand_type),
		"'");
	fp::print_line_number_with_indication_and_texts(
		error_stream,
		content_range,
		error.right_operand_type_origin,
		fp::note_string,
		"right operand type defined here");
}

void print_compile_error(
	const fs::parser::lookup_data& lookup_data,
	fs::compiler::error::internal_error_while_parsing_constant error,
	std::ostream& error_stream)
{
	error_stream << "INTERNAL COMPILER ERROR while parsing a type definition expression\n"
		"please report a bug with attached minimal source that reproduces it\n";

	const range_type content_range = lookup_data.get_range_of_whole_content();
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

}

namespace fs::print
{

void compile_error(
	const parser::lookup_data& lookup_data,
	compiler::error::error_variant error,
	std::ostream& error_stream)
{
	std::visit(
		[&](auto error) { print_compile_error(lookup_data, error, error_stream); },
		error);
}

}
