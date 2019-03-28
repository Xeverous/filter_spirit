#pragma once
#include "parser/parser.hpp"
#include "compiler/error.hpp"
#include "log/logger.hpp"

namespace fs::print
{

void compile_error(
	compiler::error::error_variant error,
	const parser::lookup_data& lookup_data,
	logger& logger);

}
