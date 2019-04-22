#pragma once

#include "fs/compiler/error.hpp"
#include "fs/parser/ast.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/lang/filter_block.hpp"
#include "fs/itemdata/types.hpp"

#include <string>
#include <vector>

namespace fs::compiler
{

[[nodiscard]]
std::variant<lang::constants_map, compiler::error::error_variant> resolve_constants(
	const std::vector<parser::ast::constant_definition>& constant_definitions,
	const itemdata::item_price_data& item_price_data);

[[nodiscard]]
std::string generate_filter(const std::vector<lang::filter_block>& blocks);

}
