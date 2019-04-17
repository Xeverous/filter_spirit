#pragma once

#include "fs/parser/error.hpp"
#include "fs/parser/parser.hpp"
#include "fs/log/logger.hpp"

namespace fs::print
{

void print_parse_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	logger& logger);

}
