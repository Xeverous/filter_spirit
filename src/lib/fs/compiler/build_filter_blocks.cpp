#include <fs/compiler/build_filter_blocks.hpp>
#include <fs/compiler/detail/add_action.hpp>
#include <fs/compiler/detail/add_conditions.hpp>
#include <fs/lang/action_set.hpp>
#include <fs/lang/condition_set.hpp>
#include <fs/lang/queries.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace
{

namespace ast = fs::parser::ast;
namespace lang = fs::lang;
namespace x3 = boost::spirit::x3;

using fs::compiler::compile_error;
using fs::compiler::detail::add_action;
using fs::compiler::detail::add_conditions;

std::optional<compile_error> apply_statements_recursively(
	lang::condition_set parent_conditions,
	lang::action_set parent_actions,
	const std::vector<ast::statement>& statements,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	std::vector<lang::filter_block>& blocks)
{
	for (const ast::statement& statement : statements) {
		auto error = statement.apply_visitor(x3::make_lambda_visitor<std::optional<compile_error>>(
			[&](const ast::action& action) {
				return add_action(action, symbols, item_price_data, parent_actions);
			},
			[&](const ast::visibility_statement& vs) {
				blocks.push_back(lang::filter_block{vs.show, parent_conditions, parent_actions});
				return std::nullopt;
			},
			[&](const ast::rule_block& nested_block) {
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::condition_set nested_conditions(parent_conditions);
				std::optional<compile_error> error = add_conditions(
					nested_block.conditions, symbols, item_price_data, nested_conditions);
				if (error)
					return error;

				return apply_statements_recursively(
					std::move(nested_conditions),
					parent_actions,
					nested_block.statements,
					symbols,
					item_price_data,
					blocks);
			}));

		if (error)
			return error;
	}

	return std::nullopt;
}

} // namespace

namespace fs::compiler {

std::variant<std::vector<lang::filter_block>, compile_error> build_filter_blocks(
	const std::vector<parser::ast::statement>& top_level_statements,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::vector<lang::filter_block> blocks;
	std::optional<compile_error> error = apply_statements_recursively(
		{}, {}, top_level_statements, symbols, item_price_data, blocks);
	if (error)
		return *std::move(error);

	return blocks;
}

}
