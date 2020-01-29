#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/price_range.hpp>
#include <fs/lang/item_price_data.hpp>

#include <variant>

namespace fs::compiler::detail
{

[[nodiscard]] std::variant<lang::price_range, compile_error>
construct_price_range(
	const parser::ast::sf::value_expression_list& arguments,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data);

}
