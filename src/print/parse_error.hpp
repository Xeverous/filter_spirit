#pragma once
#include "parser/error.hpp"
#include "parser/parser.hpp"
#include <iosfwd>

namespace fs::print
{

void print_parse_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream);

}
