#pragma once

#include "fs/parser/ast.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/lang/item_price_data.hpp"

#include <vector>
#include <optional>

namespace fs::compiler::detail
{

/*
 * determine types of expressions in an expression list
 *
 * return a vector of optionals because any expression evaluation may fail
 */
[[nodiscard]] std::vector<std::optional<lang::object_type>>
determine_types_of(
	const parser::ast::value_expression_list& expressions,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data);

}
