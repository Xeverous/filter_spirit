#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/action_set.hpp>

#include <optional>

namespace fs::compiler::detail
{

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_action(
	const parser::ast::sf::action& action,
	const lang::symbol_table& symbols,
	lang::action_set& action_set);

[[nodiscard]] std::optional<compile_error>
real_filter_add_action(
	const parser::ast::rf::action& action,
	lang::action_set& action_set);

}
