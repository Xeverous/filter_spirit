#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/item_price_data.hpp>

#include <variant>

namespace fs::compiler::detail
{

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_value_expression(
	const parser::ast::value_expression& value_expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data);

}
