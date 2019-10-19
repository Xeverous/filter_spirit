#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/condition_set.hpp>
#include <fs/lang/item_price_data.hpp>

#include <optional>

namespace fs::compiler::detail
{

[[nodiscard]] std::optional<compile_error>
add_conditions(
	const std::vector<parser::ast::condition>& conditions,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set);

}
