#pragma once

#include "fs/parser/ast.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/lang/item_price_data.hpp"
#include "fs/compiler/error.hpp"

#include <variant>

namespace fs::compiler::detail
{

[[nodiscard]]
std::variant<lang::object, error::error_variant> evaluate_value_expression(
	const parser::ast::value_expression& value_expression,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data);

}
