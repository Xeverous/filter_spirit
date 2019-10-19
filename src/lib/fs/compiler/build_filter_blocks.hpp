#pragma once
#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/filter_block.hpp>
#include <fs/lang/item_price_data.hpp>

#include <vector>
#include <optional>

namespace fs::compiler
{

[[nodiscard]] std::variant<std::vector<lang::filter_block>, compile_error>
build_filter_blocks(
	const std::vector<parser::ast::statement>& top_level_statements,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data);

}
