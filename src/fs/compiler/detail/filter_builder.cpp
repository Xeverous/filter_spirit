#include "fs/compiler/detail/filter_builder.hpp"
#include "fs/compiler/detail/actions.hpp"
#include "fs/compiler/detail/conditions.hpp"
#include "fs/lang/queries.hpp"

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <utility>

namespace fs::compiler::detail
{

namespace ast = parser::ast;
namespace x3 = boost::spirit::x3;

std::variant<std::vector<lang::filter_block>, error::error_variant> filter_builder::build_filter(
	const std::vector<parser::ast::statement>& top_level_statements,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	return filter_builder(map, item_price_data).build_filter(top_level_statements);
}

std::variant<std::vector<lang::filter_block>, error::error_variant> filter_builder::build_filter(
	const std::vector<ast::statement>& top_level_statements) &&
{
	lang::condition_set conditions;
	lang::action_set actions;
	std::optional<error::error_variant> error = build_nested(top_level_statements, conditions, actions);
	if (error)
		return *std::move(error);

	return blocks;
}

std::optional<error::error_variant> filter_builder::build_nested(
	const std::vector<ast::statement>& statements,
	lang::condition_set parent_conditions,
	lang::action_set parent_actions)
{
	for (const ast::statement& statement : statements)
	{
		auto error = statement.apply_visitor(x3::make_lambda_visitor<std::optional<error::error_variant>>(
			[&, this](const ast::action& action)
			{
				return add_action(action, map, item_price_data, parent_actions);
			},
			[&, this](const ast::visibility_statement& vs)
			{
				return handle_visibility_statement(vs, parent_conditions, parent_actions);
			},
			[&, this](const ast::rule_block& nested_block)
			{
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::condition_set nested_conditions(parent_conditions);
				std::optional<error::error_variant> error = add_conditions(nested_block.conditions, map, item_price_data, nested_conditions);
				if (error)
					return error;

				return build_nested(nested_block.statements, std::move(nested_conditions), parent_actions);
			}));

		if (error)
			return error;
	}

	return std::nullopt;
}

void filter_builder::add_block(
	bool show,
	lang::condition_set conditions,
	lang::action_set actions)
{
	blocks.push_back(lang::filter_block{show, std::move(conditions), std::move(actions)});
}

std::optional<error::error_variant> filter_builder::handle_visibility_statement(
	const ast::visibility_statement& vs,
	const lang::condition_set& parent_conditions,
	const lang::action_set& parent_actions)
{
	add_block(vs.show, parent_conditions, parent_actions);
	return std::nullopt;
}

}
