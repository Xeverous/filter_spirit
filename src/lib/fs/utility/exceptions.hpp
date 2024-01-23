#pragma once

#include <stdexcept>
#include <string>

namespace fs
{

class unhandled_switch_case : public std::logic_error
{
public:
	unhandled_switch_case(const char* file, int line, const char* func_name)
	: std::logic_error(std::string("unhandled switch case at ") + file + ":" + std::to_string(line) + " in function " + func_name)
	{}
};

}
