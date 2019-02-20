#include "compiler/filter_builder.hpp"
#include "compiler/actions.hpp"
#include "compiler/conditions.hpp"
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <utility>

namespace fs::compiler
{

namespace ast = parser::ast;
namespace x3 = boost::spirit::x3;

std::optional<error::error_variant> filter_builder::build_filter()
{
	lang::condition_set conditions;
	lang::action_set actions;
	return build_nested(statements, conditions, actions);
}

std::optional<error::error_variant> filter_builder::build_nested(
	const std::vector<ast::statement>& statements,
	lang::condition_set parent_conditions,
	lang::action_set parent_actions)
{
	for (const ast::statement& statement : statements)
	{
		auto error = statement.apply_visitor(x3::make_lambda_visitor<std::optional<error::error_variant>>(
			[](const ast::action& action)
			{
				return add_action(action, map, parent_actions);
			},
			[&, this](const ast::visibility_statement& vs)
			{
				add_block(vs.show, parent_conditions, parent_actions);
				return std::nullopt;
			},
			[&, this](const ast::rule_block& nested_block)
			{
				std::optional<error::error_variant> error = add_conditions(nested_block.conditions, map, parent_conditions);
				if (error)
					return *error;

				return build_nested(nested_block.statements, parent_conditions, parent_actions);
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
