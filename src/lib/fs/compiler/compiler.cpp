#include <fs/compiler/compiler.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/compiler/detail/conditions.hpp>
#include <fs/lang/item_filter.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <optional>

namespace
{

using namespace fs;
using namespace fs::compiler;

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

// ---- resolve_spirit_filter_symbols helpers ----

/*
 * core entry point into adding constants
 *
 * task:
 * Add object only if valid, error with appropriate information otherwise; in any case
 * after either successful addition or any error return immediately.
 *
 * flow:
 * - check that name is not already taken and error if so
 *   (it's impossible to have multiple objects with the same name)
 *   (as of now, filter's language has no scoping/name shadowing)
 * - convert expression to language object and proceed
 */
[[nodiscard]] std::optional<compile_error>
add_constant_from_definition(
	const ast::sf::constant_definition& def,
	const lang::item_price_data& item_price_data,
	lang::symbol_table& symbols)
{
	const ast::sf::identifier& wanted_name = def.name;
	const ast::sf::value_expression& value_expression = def.value;

	if (const auto it = symbols.find(wanted_name.value); it != symbols.end()) {
		const lang::position_tag place_of_original_name = parser::get_position_info(it->second.name_origin);
		const lang::position_tag place_of_duplicated_name = parser::get_position_info(wanted_name);
		return errors::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	std::variant<lang::object, compile_error> expr_result =
		detail::evaluate_value_expression(value_expression, symbols, item_price_data);

	if (std::holds_alternative<compile_error>(expr_result))
		return std::get<compile_error>(std::move(expr_result));

	const auto pair = symbols.emplace(
		wanted_name.value,
		lang::named_object{
			std::get<lang::object>(std::move(expr_result)),
			parser::get_position_info(wanted_name)});
	assert(pair.second); // C++20: use [[assert]]
	(void) pair; // ignore insertion result in release builds
	return std::nullopt;
}

// ---- compile_spirit_filter helpers ----

std::optional<compile_error> apply_statements_recursively(
	lang::condition_set parent_conditions,
	lang::action_set parent_actions,
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	std::vector<lang::item_filter_block>& blocks)
{
	for (const ast::sf::statement& statement : statements) {
		auto error = statement.apply_visitor(x3::make_lambda_visitor<std::optional<compile_error>>(
			[&](const ast::sf::action& action) {
				return detail::spirit_filter_add_action(action, symbols, item_price_data, parent_actions);
			},
			[&](const ast::sf::visibility_statement& vs) {
				blocks.push_back(lang::item_filter_block{
					lang::item_visibility{vs.show, parser::get_position_info(vs)},
					parent_conditions,
					parent_actions});
				return std::nullopt;
			},
			[&](const ast::sf::rule_block& nested_block) {
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::condition_set nested_conditions(parent_conditions);
				std::optional<compile_error> error = detail::spirit_filter_add_conditions(
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

namespace fs::compiler
{

std::variant<lang::symbol_table, compile_error>
resolve_spirit_filter_symbols(
	const std::vector<parser::ast::sf::definition>& definitions,
	const lang::item_price_data& item_price_data)
{
	lang::symbol_table symbols;

	for (const auto& def : definitions) {
		std::optional<compile_error> error = add_constant_from_definition(def.def, item_price_data, symbols);

		if (error)
			return *std::move(error);
	}

	return symbols;
}

std::variant<lang::item_filter, compile_error>
compile_spirit_filter(
	const std::vector<parser::ast::sf::statement>& statements,
	const lang::symbol_table& symbols,
	const lang::item_price_data& ipd)
{
	std::vector<lang::item_filter_block> blocks;
	std::optional<compile_error> error = apply_statements_recursively(
		{}, {}, statements, symbols, ipd, blocks);
	if (error)
		return *std::move(error);

	return lang::item_filter{ blocks };
}

std::variant<lang::item_filter, compile_error>
compile_real_filter(
	const parser::ast::rf::ast_type& ast)
{
	lang::item_filter filter;
	filter.blocks.reserve(ast.size());

	for (const auto& block : ast) {
		lang::item_filter_block filter_block;

		for (const auto& rule : block.rules) {
			auto error = rule.apply_visitor(x3::make_lambda_visitor<std::optional<compile_error>>(
				[&](const parser::ast::rf::action& a) {
					return detail::real_filter_add_action(a, filter_block.actions);
				},
				[&](const parser::ast::rf::condition& c) {
					return detail::real_filter_add_condition(c, filter_block.conditions);
				}
			));

			if (error)
				return *std::move(error);

		}

		filter_block.visibility = lang::item_visibility{block.visibility.show, parser::get_position_info(block.visibility)};
		filter.blocks.push_back(std::move(filter_block));
	}

	return filter;
}

}
