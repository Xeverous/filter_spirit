#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/condition_set.hpp>

namespace fs::compiler::detail
{

[[nodiscard]] bool
spirit_filter_add_conditions(
	settings st,
	const std::vector<parser::ast::sf::condition>& conditions,
	const lang::symbol_table& symbols,
	lang::spirit_condition_set& condition_set,
	diagnostics_container& diagnostics);

[[nodiscard]] bool
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::condition_set& condition_set,
	diagnostics_container& diagnostics);

}
