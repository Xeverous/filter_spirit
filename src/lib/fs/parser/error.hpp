#pragma once

#include <string>

namespace fs::parser
{

struct parse_error
{
	const char* error_place;        // this IS an input iterator, NOT a null-terminated C-string!
	const char* backtracking_place; // this IS an input iterator, NOT a null-terminated C-string!
	std::string what_was_expected;
};

}
