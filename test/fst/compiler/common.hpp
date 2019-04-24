#pragma once

#include "fs/lang/constants_map.hpp"
#include "fs/parser/parser.hpp"
#include "fs/parser/ast.hpp"
#include "fs/compiler/error.hpp"

#include <string_view>
#include <variant>

namespace fsut::compiler
{

class compiler_fixture
{
protected:
	fs::parser::parse_data parse(std::string_view input);

	std::variant<fs::lang::constants_map, fs::compiler::error::error_variant>
	resolve_constants(const std::vector<fs::parser::ast::constant_definition>& defs);
};

}
