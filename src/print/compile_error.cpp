#include "print/compile_error.hpp"
#include "print/generic.hpp"
#include "compiler/error.hpp"

namespace
{

namespace fp = fs::print;
using iterator_type = fp::iterator_type;
using range_type = fp::range_type;


void compile_error(
	std::ostream& os,
	range_type content,
	range_type error_range,
	std::string_view message)
{
	error_range = fp::skip_whitespace(error_range, content.end());

	fp::print_line_number_with_texts(
		os,
		fp::count_line_number(content.begin(), error_range.begin()),
		fp::compiler_error_string,
		message);

	fp::print_line_with_indicator(os, content, error_range);
}

void print_compile_error(const fs::parser::lookup_data& /* lookup_data */, fs::compiler::error::no_error /* error */, std::ostream& error_stream)
{
	error_stream << "(no error)\n"; // TODO refactor so that there is no no_error type, then remove this overload
}

void print_compile_error(const fs::parser::lookup_data& lookup_data, fs::compiler::error::name_already_exists error, std::ostream& error_stream)
{
	error_stream << "name already exists\n"; // TODO add info
}

void print_compile_error(const fs::parser::lookup_data& lookup_data, fs::compiler::error::no_such_name error, std::ostream& error_stream)
{
	compile_error(error_stream, lookup_data.get_range_of_whole_content(), error.name_origin, "no such name exists");
}

void print_compile_error(const fs::parser::lookup_data& lookup_data, fs::compiler::error::type_mismatch error, std::ostream& error_stream)
{
	const range_type content_range = lookup_data.get_range_of_whole_content();
	const range_type value_origin_range = fp::skip_whitespace(error.right_operand_value_origin, content_range.end());

	fp::print_line_number_with_texts(
		error_stream,
		fp::count_line_number(content_range.begin(), value_origin_range.begin()),
		fp::compiler_error_string,
		"type mismatch in expression, operands of types '",
		fs::lang::to_string(error.left_operand_type),
		"' and '",
		fs::lang::to_string(error.right_operand_type),
		"'");
	fp::print_line_with_indicator(error_stream, content_range, value_origin_range);

	const range_type type_origin_range = fp::skip_whitespace(error.right_operand_type_origin, content_range.end());

	fp::print_line_number_with_texts(
		error_stream,
		fp::count_line_number(content_range.begin(), type_origin_range.begin()),
		fp::note_string,
		"right operand type defined here");
	fp::print_line_with_indicator(error_stream, content_range, type_origin_range);
}

void print_compile_error(const fs::parser::lookup_data& lookup_data, fs::compiler::error::internal_error /* error */, std::ostream& error_stream)
{
	error_stream << "internal compiler error\nplease report a bug with attached minimal source that reproduces it\n";
}

}

namespace fs::print
{

void compile_error(const parser::lookup_data& lookup_data, compiler::error::error_variant error, std::ostream& error_stream)
{
	std::visit(
		[&](auto error) { print_compile_error(lookup_data, error, error_stream); },
		error);
}

}
