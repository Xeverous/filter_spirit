#pragma once

#include <stdexcept>

namespace fs::itemdata
{

class json_parse_error : public std::runtime_error
{
public:
	explicit json_parse_error(const std::string& what)
	: std::runtime_error(what) {}
	explicit json_parse_error(const char* what)
	: std::runtime_error(what) {}
};

class unknown_item_variation : public json_parse_error
{
public:
	unknown_item_variation(const std::string& what)
	: json_parse_error(what) {}
};

}
