#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>
#include <fs/lang/action_set.hpp>

namespace fs::compiler::detail
{

[[nodiscard]] bool
spirit_filter_add_action(
	settings st,
	const parser::ast::sf::action& action,
	const symbol_table& symbols,
	lang::action_set& action_set,
	diagnostics_container& diagnostics);

[[nodiscard]] bool
real_filter_add_action(
	settings st,
	const parser::ast::rf::action& action,
	lang::action_set& action_set,
	diagnostics_container& diagnostics);

}
