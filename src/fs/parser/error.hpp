#pragma once

#include "fs/parser/detail/config.hpp"

#include <string>

namespace fs::parser
{

struct parse_error
{
	const char* error_place;
	const char* backtracking_place;
	std::string what_was_expected;
};

}
