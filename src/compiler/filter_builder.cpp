#include "compiler/filter_builder.hpp"
#include "compiler/actions.hpp"
#include "compiler/conditions.hpp"

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <utility>

namespace fs::compiler
{

namespace ast = parser::ast;
namespace x3 = boost::spirit::x3;

std::variant<std::vector<lang::filter_block>, error::error_variant> filter_builder::build_filter(
	const std::vector<parser::ast::statement>& top_level_statements,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	filter_builder fb(top_level_statements, map, item_price_data);
	return fb.build_filter();
}

std::variant<std::vector<lang::filter_block>, error::error_variant> filter_builder::build_filter()
{
	lang::condition_set conditions;
	lang::action_set actions;
	std::optional<error::error_variant> error = build_nested(statements, conditions, actions);
	if (error)
		return *error;

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
				add_block(vs.show, parent_conditions, parent_actions);
				return std::nullopt;
			},
			[&, this](const ast::rule_block& nested_block)
			{
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::condition_set nested_conditions(parent_conditions);
				std::optional<error::error_variant> error = add_conditions(nested_block.conditions, map, item_price_data, nested_conditions);
				if (error.has_value())
					return error;

				return build_nested(nested_block.statements, std::move(nested_conditions), parent_actions);
			}));

		if (error)
			return *error;
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

}
