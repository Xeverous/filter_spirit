#pragma once
#include "fs/parser/parser.hpp"
#include "fs/compiler/error.hpp"
#include "fs/log/logger.hpp"

namespace fs::print
{

void compile_error(
	compiler::error::error_variant error,
	const parser::lookup_data& lookup_data,
	logger& logger);

}
