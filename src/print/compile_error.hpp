#pragma once
#include "parser/parser.hpp"
#include "compiler/error.hpp"
#include <iosfwd>

namespace fs::print
{

void compile_error(
	compiler::error::error_variant error,
	parser::range_type content_range,
	std::ostream& error_stream);

}
