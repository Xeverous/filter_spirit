#pragma once

#include "fs/parser/config.hpp"

#include <string>

namespace fs::parser
{

struct parse_error
{
	iterator_type error_place;
	std::string what_was_expected;
};

}
