#pragma once
#include "parser/parser.hpp"
#include "compiler/error.hpp"
#include <iosfwd>

namespace fs::print
{

void compile_error(const parser::parse_result& parse_data, compiler::error::error_variant error, std::ostream& error_stream);

}
