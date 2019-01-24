#include "print/compile_error.hpp"
#include "print/parse_error.hpp"
#include "parser/parser.hpp"
#include "compiler/error.hpp"
#include <iostream>

namespace
{

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::no_error /* error */, std::ostream& error_stream)
{
	error_stream << "(no error)\n";
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::name_already_exists error, std::ostream& error_stream)
{
	fs::print::parse_error(error_stream, parse_data.get_range_of_all(), error.place_of_error, "name already exists"); // TODO don't redirect
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::no_such_name error, std::ostream& error_stream)
{
	fs::print::parse_error(error_stream, parse_data.get_range_of_all(), error.place_of_error, "no such name exists");
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::type_mismatch /* error */, std::ostream& error_stream)
{
	error_stream << "type mismatch\n";
}

void print_compile_error(const fs::parser::parse_result& parse_data, fs::compiler::error::internal_error /* error */, std::ostream& error_stream)
{
	error_stream << "internal compiler error\nplease report a bug with attached minimal source that reproduces it\n";
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
