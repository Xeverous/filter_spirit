#pragma once

#include <fs/parser/parser.hpp>
#include <fs/compiler/error.hpp>
#include <fs/log/logger.hpp>

namespace fs::compiler
{

void print_error(
	const compiler::compile_error& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger);

}
