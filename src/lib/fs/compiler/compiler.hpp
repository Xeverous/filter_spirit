#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>

#include <variant>

namespace fs::compiler
{

[[nodiscard]] std::variant<lang::symbol_table, compile_error>
resolve_spirit_filter_symbols(
	const std::vector<parser::ast::sf::definition>& definitions);

[[nodiscard]] std::variant<lang::item_filter, compile_error>
compile_spirit_filter(
	const std::vector<parser::ast::sf::statement>& statements,
	const lang::symbol_table& symbols);

[[nodiscard]] std::variant<lang::item_filter, compile_error>
compile_real_filter(
	const parser::ast::rf::ast_type& ast);

}
