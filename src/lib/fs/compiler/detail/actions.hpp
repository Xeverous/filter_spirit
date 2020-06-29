#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/outcome.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/action_set.hpp>

#include <optional>

namespace fs::compiler::detail
{

[[nodiscard]] outcome<>
spirit_filter_add_action(
	settings st,
	const parser::ast::sf::action& action,
	const lang::symbol_table& symbols,
	lang::action_set& action_set);

[[nodiscard]] outcome<>
real_filter_add_action(
	settings st,
	const parser::ast::rf::action& action,
	lang::action_set& action_set);

}
