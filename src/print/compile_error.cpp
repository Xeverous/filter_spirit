#include "print/compile_error.hpp"
#include "print/parse_error.hpp"
#include "parser/parser.hpp"
#include "compiler/error.hpp"
#include <iostream>

namespace
{

using iterator_type = fs::parser::iterator_type;
using range_type = fs::parser::range_type;

template <typename OutputStream>
void compile_error(
	OutputStream& os,
	std::string_view message)
{
	os << message << '\n';
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::no_error /* error */, std::ostream& error_stream)
{
	compile_error(error_stream, "(no error)");
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::name_already_exists error, std::ostream& error_stream)
{
	compile_error(error_stream, "name already exists"); // TODO add info
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::no_such_name error, std::ostream& error_stream)
{
	compile_error(error_stream, "no such name exists"); // TODO add info
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::type_mismatch /* error */, std::ostream& error_stream)
{
	compile_error(error_stream, "type mismatch\n"); // TODO add info
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::internal_error /* error */, std::ostream& error_stream)
{
	compile_error(error_stream, "internal compiler error\nplease report a bug with attached minimal source that reproduces it");
}

}

namespace fs::print
{

void compile_error(const parser::parse_result& parse_data, compiler::error::error_variant error, std::ostream& error_stream)
{
	std::visit(
		[&](auto error) { print_compile_error(parse_data, error, error_stream); },
		error);
}

}
