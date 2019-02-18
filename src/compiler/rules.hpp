#pragma once
#include "compiler/error.hpp"
#include "compiler/compiler.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include "lang/constants_map.hpp"
#include "lang/action_set.hpp"
#include "lang/condition_set.hpp"
#include "lang/filter_block.hpp"
#include <variant>

namespace fs::compiler
{

[[nodiscard]]
std::variant<std::vector<lang::filter_block>, error::error_variant> compile_rules(
	const std::vector<parser::ast::action>& top_level_actions,
	const std::vector<parser::ast::rule_block>& top_level_blocks,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
std::optional<error::error_variant> add_rules_to_filter(
	lang::condition_set parent_condition_set,
	lang::action_set parent_action_set,
	const parser::ast::rule_block& block,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data,
	std::vector<lang::filter_block>& filter_content);

}
