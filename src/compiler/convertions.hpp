#pragma once
#include "lang/types.hpp"
#include "compiler/error.hpp"
#include <string>
#include <vector>
#include <variant>

namespace fs::compiler
{

std::variant<std::vector<std::string>, error::error_variant> array_to_strings(
	lang::array_object array);

}
