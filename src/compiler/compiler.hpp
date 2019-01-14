#pragma once
#include "parser/parser.hpp"
#include "compiler/ast.hpp"
#include "compiler/error.hpp"
#include <string>
#include <unordered_map>
#include <variant>

namespace fs::compiler
{

using constants_map = std::unordered_map<std::string, ast::object_type>;

std::variant<constants_map, error_type> parse_constants(const parser::ast::ast_type& ast);

}
