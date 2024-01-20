#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>
#include <fs/lang/condition_set.hpp>

namespace fs::compiler::detail
{

[[nodiscard]] bool
spirit_filter_add_condition(
	settings st,
	const parser::ast::sf::condition& condition,
	const symbol_table& symbols,
	lang::spirit_condition_set& condition_set,
	diagnostics_store& diagnostics);

[[nodiscard]] bool
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::condition_set& condition_set,
	diagnostics_store& diagnostics);

}
