#pragma once
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include <string>
#include <optional>
#include <iosfwd>

namespace fs::compiler
{

[[nodiscard]]
bool compile(const std::string& file_content, std::ostream& error_stream);

}
