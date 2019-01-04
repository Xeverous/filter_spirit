#pragma once

#include "grammar.hpp"

namespace fs::parser
{

fs::ast::ast_type parse(const std::string& source, position_cache& positions);

}
