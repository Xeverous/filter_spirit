#pragma once
#include "compiler/error.hpp"
#include "lang/constants_map.hpp"
#include "lang/action_set.hpp"
#include "lang/condition_set.hpp"
#include "lang/filter_block.hpp"
#include "parser/parser.hpp"
#include "parser/ast.hpp"
#include <utility>
#include <vector>
#include <optional>

namespace fs::compiler
{

class filter_builder
{
public:
	filter_builder(
		std::vector<parser::ast::statement>&& top_level_statements,
		lang::constants_map&& map,
		parser::lookup_data&& lookup_data)
	: statements(std::move(top_level_statements)),
	  map(std::move(map)),
	  lookup_data(std::move(lookup_data))
	{}

	[[nodiscard]]
	std::optional<error::error_variant> build_filter();

private:
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

	const std::vector<parser::ast::statement> statements;
	const lang::constants_map map;
	const parser::lookup_data lookup_data;

	std::vector<lang::filter_block> blocks;
};

}
