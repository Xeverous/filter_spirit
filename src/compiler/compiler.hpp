#pragma once
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include <string>
#include <optional>
#include <iosfwd>
#include <unordered_map>

namespace fs::compiler
{

using constants_map = std::unordered_map<std::string, lang::object>;

[[nodiscard]]
bool compile(const std::string& file_content, std::ostream& error_stream);

}
