#pragma once

#include <fs/parser/error.hpp>
#include <fs/parser/parser.hpp>

namespace fs::log { class logger; }

namespace fs::parser
{

void print_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	log::logger& logger);

}
