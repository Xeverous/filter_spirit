#pragma once

#include <fs/parser/parser.hpp>
#include <fs/compiler/error.hpp>

namespace fs::log { class logger; }

namespace fs::compiler
{

void print_error(
	const compiler::compile_error& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger);

}
