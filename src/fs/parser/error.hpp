#pragma once

#include "fs/parser/detail/config.hpp"

#include <string>

namespace fs::parser
{

struct parse_error
{
	detail::iterator_type parsed_place;
	detail::iterator_type error_place; // TODO this exposes internals
	std::string what_was_expected;
};

}
