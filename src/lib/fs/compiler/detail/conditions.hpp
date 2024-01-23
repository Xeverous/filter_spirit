#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>
#include <fs/compiler/detail/types.hpp>
#include <fs/lang/conditions.hpp>

namespace fs::compiler::detail
{

[[nodiscard]] bool
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::official_conditions& block_conditions,
	diagnostics_store& diagnostics);

[[nodiscard]] bool
spirit_filter_add_condition(
	settings st,
	const parser::ast::sf::condition& condition,
	const symbol_table& symbols,
	spirit_protoconditions& block_conditions,
	diagnostics_store& diagnostics);

}
