#pragma once
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include "lang/types.hpp"
#include <string>

namespace fs::compiler
{

[[nodiscard]]
bool process_input(const std::string& input, std::ostream& error_stream);

}
