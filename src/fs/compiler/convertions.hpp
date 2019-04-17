#pragma once
#include "fs/lang/types.hpp"
#include "fs/compiler/error.hpp"
#include <string>
#include <vector>
#include <variant>

namespace fs::compiler
{

std::variant<std::vector<std::string>, error::error_variant> array_to_strings(
	lang::array_object array);

}
