#pragma once

#include "parser/grammar.hpp"

namespace fs::parser
{

ast::ast_type parse(const std::string& source, bool& result);

void print_ast(const ast::ast_type& ast);

}
