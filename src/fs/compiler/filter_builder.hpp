#pragma once
#include "fs/compiler/error.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/lang/action_set.hpp"
#include "fs/lang/condition_set.hpp"
#include "fs/lang/filter_block.hpp"
#include "fs/parser/parser.hpp"
#include "fs/parser/ast.hpp"
#include "fs/itemdata/types.hpp"
#include <utility>
#include <vector>
#include <optional>

namespace fs::compiler
{

class filter_builder
{
public:
	[[nodiscard]] static
	std::variant<std::vector<lang::filter_block>, error::error_variant> build_filter(
		const std::vector<parser::ast::statement>& top_level_statements,
		const lang::constants_map& map,
		const itemdata::item_price_data& item_price_data);

private:
	filter_builder(
		const lang::constants_map& map,
		const itemdata::item_price_data& item_price_data)
	: map(map), item_price_data(item_price_data)
	{}

	[[nodiscard]]
	std::variant<std::vector<lang::filter_block>, error::error_variant> build_filter(
		const std::vector<parser::ast::statement>& top_level_statements);

	/*
	 * copies of actions and conditions are intentional
	 *
	 * we rely on recursive call stack that matches 1:1
	 * with language block nesting - this way there is no
	 * need to rollback rules that run out of scope (they
	 * simply return to parent function call with old instances)
	 */
	[[nodiscard]]
	std::optional<error::error_variant> build_nested(
		const std::vector<parser::ast::statement>& statements,
		lang::condition_set parent_conditions,
		lang::action_set parent_actions);

	void add_block(
		bool show,
		lang::condition_set conditions,
		lang::action_set actions);

	[[nodiscard]]
	std::optional<error::error_variant> handle_visibility_statement(
		const parser::ast::visibility_statement& vs,
		const lang::condition_set& parent_conditions,
		const lang::action_set& parent_actions);

	[[nodiscard]]
	std::optional<error::error_variant> handle_advanced_price_range_query(
		bool show,
		const parser::ast::advanced_price_range_query& query,
		const lang::condition_set& parent_conditions,
		const lang::action_set& parent_actions);

	const lang::constants_map& map;
	const itemdata::item_price_data& item_price_data;

	std::vector<lang::filter_block> blocks;
};

}
