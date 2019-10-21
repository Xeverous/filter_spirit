#pragma once

#include <fs/compiler/error.hpp>
#include <fs/parser/ast.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/item_price_data.hpp>

#include <string>
#include <vector>

namespace fs::compiler
{

[[nodiscard]] std::variant<lang::symbol_table, compile_error>
resolve_symbols(
	const std::vector<parser::ast::definition>& definitions,
	const lang::item_price_data& item_price_data);

}
