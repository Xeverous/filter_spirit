#pragma once

#include <stdexcept>

namespace fs::itemdata
{

// use this type to indicate logic errors (eg item has missing / invalid field type)
// do not use this type for syntax errors (use nlohmann types instead)
class json_parse_error : public std::runtime_error
{
public:
	explicit json_parse_error(const std::string& what)
	: std::runtime_error(what) {}
	explicit json_parse_error(const char* what)
	: std::runtime_error(what) {}
};

}
