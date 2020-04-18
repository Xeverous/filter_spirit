#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/outcome.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/condition_set.hpp>

#include <optional>

namespace fs::compiler::detail
{

[[nodiscard]] outcome<>
spirit_filter_add_conditions(
	settings st,
	const std::vector<parser::ast::sf::condition>& conditions,
	const lang::symbol_table& symbols,
	lang::spirit_condition_set& condition_set);

[[nodiscard]] outcome<>
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::condition_set& condition_set);

}
