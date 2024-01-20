#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>

#include <boost/optional.hpp>

#include <vector>

namespace fs::compiler
{

[[nodiscard]] boost::optional<symbol_table>
resolve_spirit_filter_symbols(
	settings st,
	const std::vector<parser::ast::sf::definition>& definitions,
	diagnostics_store& diagnostics);

[[nodiscard]] boost::optional<lang::spirit_item_filter>
compile_spirit_filter_statements(
	settings st,
	const std::vector<parser::ast::sf::statement>& statements,
	const symbol_table& symbols,
	diagnostics_store& diagnostics);

[[nodiscard]] boost::optional<lang::item_filter>
compile_real_filter(
	settings st,
	const parser::ast::rf::ast_type& ast,
	diagnostics_store& diagnostics);

}
