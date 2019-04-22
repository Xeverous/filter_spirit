#pragma once

#include "fs/core/version.hpp"

#include <string>

namespace fsut::parser
{

inline
const std::string& minimal_input()
{
	namespace v = fs::core::version;
	static const std::string str =
		"version: " + std::to_string(v::major) + "." + std::to_string(v::minor) + "." + std::to_string(v::patch) + "\n"
		"config: {}";
	return str;
}

}
