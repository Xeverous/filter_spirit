#include <fs/compiler/compiler.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/compiler/detail/conditions.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/item_classes.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/string_helpers.hpp>

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
[[nodiscard]] outcome<>
add_constant_from_definition(
	settings st,
	const ast::sf::constant_definition& def,
	lang::symbol_table& symbols)
{
	const auto wanted_name_origin = parser::position_tag_of(def.value_name);
	const std::string& wanted_name = def.value_name.value.value;

	if (const auto it = symbols.find(wanted_name); it != symbols.end()) {
		const lang::position_tag place_of_original_name = parser::position_tag_of(it->second.name_origin);
		return error(errors::name_already_exists{wanted_name_origin, place_of_original_name});
	}

	return detail::evaluate_value_expression(st, def.value, symbols)
		.map_result([&](lang::object obj) {
			const bool success = symbols.emplace(
				wanted_name,
				lang::named_object{std::move(obj), wanted_name_origin}).second;
			FS_ASSERT_MSG(success, "Insertion should succeed.");
			(void) success; // shut unused variable warning
		});
}

// ---- compile_spirit_filter_statements helpers ----

bool condition_contains(const lang::strings_condition& condition, std::string_view fragment)
{
	for (const auto& str : condition.strings) {
		if (utility::contains(str.value, fragment))
			return true;
	}

	return false;
}

[[nodiscard]] outcome<>
verify_autogen_cards(
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	auto result = outcome<>::success();

	if (conditions.class_.has_value()) {
		if (!condition_contains(*conditions.class_, fs::lang::classes::cards)) {
			result.logs().emplace_back(error(errors::autogen_error{
				errors::autogen_error_cause::invalid_class_condition,
				autogen_origin,
				(*conditions.class_).origin,
				visibility_origin}));
			result.destroy_result();
		}
	}

	if (conditions.prophecy.has_value()) {
		result.logs().emplace_back(error(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen_origin,
			(*conditions.prophecy).origin,
			visibility_origin}));
		result.destroy_result();
	}

	if (conditions.gem_level.has_bound()) {
		result.logs().emplace_back(error(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen_origin,
			*conditions.gem_level.first_origin(),
			visibility_origin}));
		result.destroy_result();
	}

	if (conditions.map_tier.has_bound()) {
		result.logs().emplace_back(error(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen_origin,
			*conditions.map_tier.first_origin(),
			visibility_origin}));
		result.destroy_result();
	}

	return result;
}

[[nodiscard]] outcome<>
verify_autogen_currency(
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	auto result = outcome<>::success();

	if (conditions.class_.has_value()) {
		if (!condition_contains(*conditions.class_, "Currency")) {
			result.logs().emplace_back(error(errors::autogen_error{
				errors::autogen_error_cause::invalid_class_condition,
				autogen_origin,
				(*conditions.class_).origin,
				visibility_origin}));
			result.destroy_result();
		}
	}

	return result;
}

[[nodiscard]] outcome<>
verify_autogen_uniques(
	lang::autogen_condition autogen,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	auto result = outcome<>::success();

	if (!conditions.rarity.includes(lang::rarity_type::unique)) {
		result.logs().emplace_back(error(errors::autogen_error{
			errors::autogen_error_cause::invalid_rarity_condition,
			autogen.origin,
			*conditions.rarity.first_origin(),
			visibility_origin}));
		result.destroy_result();
	}

	if (conditions.stack_size.has_bound()) {
		result.logs().emplace_back(error(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			*conditions.stack_size.first_origin(),
			visibility_origin}));
		result.destroy_result();
	}

	if (conditions.gem_level.has_bound()) {
		result.logs().emplace_back(error(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			*conditions.gem_level.first_origin(),
			visibility_origin}));
		result.destroy_result();
	}

	return result;
}

[[nodiscard]] outcome<>
verify_autogen(
	lang::autogen_condition autogen,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	auto result = outcome<>::success();

	// all autogens require empty BaseType
	if (conditions.base_type.has_value()) {
		result.logs().emplace_back(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			(*conditions.base_type).origin,
			visibility_origin});
		result.destroy_result();
	}

	using cat_t = lang::item_category;

	// all autogens except prophecies require empty Prophecy
	if (autogen.category != cat_t::prophecies && conditions.prophecy.has_value()) {
		result.logs().emplace_back(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			(*conditions.prophecy).origin,
			visibility_origin});
		result.destroy_result();
	}

	// all autogens except uniques_maps_* require empty MapTier
	if (const bool is_map_autogen =
			autogen.category == cat_t::uniques_maps_unambiguous ||
			autogen.category == cat_t::uniques_maps_ambiguous;
		is_map_autogen && conditions.map_tier.has_bound())
	{
		result.logs().emplace_back(errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			*conditions.map_tier.first_origin(),
			visibility_origin});
		result.destroy_result();
	}

	switch (autogen.category) {
		case cat_t::cards:
			return std::move(result).merge_with(verify_autogen_cards(autogen.origin, conditions, visibility_origin));

		case cat_t::scarabs:
		case cat_t::incubators:
			return result; // TODO add checking

		case cat_t::prophecies:
		case cat_t::essences:
		case cat_t::fossils:
		case cat_t::resonators:
		case cat_t::oils:
		case cat_t::catalysts:
			return std::move(result).merge_with(verify_autogen_currency(autogen.origin, conditions, visibility_origin));

		case cat_t::uniques_eq_unambiguous:
		case cat_t::uniques_eq_ambiguous:
		case cat_t::uniques_flasks_unambiguous:
		case cat_t::uniques_flasks_ambiguous:
		case cat_t::uniques_jewels_unambiguous:
		case cat_t::uniques_jewels_ambiguous:
		case cat_t::uniques_maps_unambiguous:
		case cat_t::uniques_maps_ambiguous:
			return std::move(result).merge_with(verify_autogen_uniques(autogen, conditions, visibility_origin));
	}

	// failed to cover given category - add internal error and return failure
	result.logs().emplace_back(error(errors::internal_compiler_error{
		errors::internal_compiler_error_cause::spirit_filter_verify_autogen_conditions,
		autogen.origin}));
	result.destroy_result();
	return result;
}

[[nodiscard]] lang::item_visibility
evaluate(parser::ast::common::visibility_statement visibility)
{
	return lang::item_visibility{
		visibility.show,
		parser::position_tag_of(visibility)
	};
}

[[nodiscard]] outcome<lang::spirit_item_filter_block>
make_spirit_filter_block(
	settings /* st */,
	lang::item_visibility visibility,
	lang::spirit_condition_set conditions,
	lang::action_set actions,
	lang::block_continuation continuation)
{
	if (conditions.price.has_bound() && !conditions.autogen.has_value()) {
		std::optional<lang::position_tag> price_first_origin = conditions.price.first_origin();
		FS_ASSERT_MSG(
			price_first_origin.has_value(),
			"Price condition has a bound. It must have at least 1 origin.");
		return error(errors::price_without_autogen{
			visibility.origin,
			*price_first_origin,
			conditions.price.second_origin()});
	}

	if (conditions.autogen) {
		return verify_autogen(*conditions.autogen, conditions.conditions, visibility.origin)
			.map_result<lang::spirit_item_filter_block>([&]() {
				return lang::spirit_item_filter_block{
					lang::item_filter_block{
						visibility,
						std::move(conditions.conditions),
						std::move(actions),
						continuation
					},
					lang::autogen_extension{conditions.price, *conditions.autogen}};
			});
	}

	return lang::spirit_item_filter_block{
		lang::item_filter_block{
			visibility,
			std::move(conditions.conditions),
			std::move(actions),
			continuation
		},
		std::nullopt};
}

lang::block_continuation
to_block_continuation(
	boost::optional<ast::common::continue_statement> statement)
{
	if (!statement)
		return lang::block_continuation{false, lang::position_tag{}};

	return lang::block_continuation{true, parser::position_tag_of(*statement)};
}

outcome<>
compile_statements_recursively(
	settings st,
	lang::spirit_condition_set parent_conditions,
	lang::action_set parent_actions,
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols,
	std::vector<lang::spirit_item_filter_block>& blocks)
{
	log_container logs;

	for (const ast::sf::statement& statement : statements) {
		statement.apply_visitor(x3::make_lambda_visitor<outcome<>>(
			[&](const ast::sf::action& action) {
				return detail::spirit_filter_add_action(st, action, symbols, parent_actions);
			},
			[&](const ast::sf::behavior_statement& bs) {
				return make_spirit_filter_block(
					st,
					evaluate(bs.visibility),
					parent_conditions,
					parent_actions,
					to_block_continuation(bs.continue_))
				.map_result([&](lang::spirit_item_filter_block block) {
					blocks.push_back(std::move(block));
				});
			},
			[&](const ast::sf::rule_block& nested_block) {
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::spirit_condition_set nested_conditions(parent_conditions);

				return detail::spirit_filter_add_conditions(
					st, nested_block.conditions, symbols, nested_conditions)
					.map_result([&]() {
						return compile_statements_recursively(
							st,
							std::move(nested_conditions),
							parent_actions,
							nested_block.statements,
							symbols,
							blocks);
					});
			}))
		.move_logs_to(logs);

		if (!should_continue(st.error_handling, logs))
			return logs;
	}

	return outcome<>::success(logs);
}

} // namespace

namespace fs::compiler
{

outcome<lang::symbol_table>
resolve_spirit_filter_symbols(
	settings st,
	const std::vector<parser::ast::sf::definition>& definitions)
{
	lang::symbol_table symbols;
	log_container logs;

	for (const auto& def : definitions) {
		add_constant_from_definition(st, def.def, symbols).move_logs_to(logs);

		if (!should_continue(st.error_handling, logs))
			return logs;
	}

	return {std::move(symbols), std::move(logs)};
}

outcome<lang::spirit_item_filter>
compile_spirit_filter_statements(
	settings st,
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols)
{
	std::vector<lang::spirit_item_filter_block> blocks;
	// start with empty conditions and actions
	// thats the default state before any nesting
	outcome<lang::spirit_item_filter> result = compile_statements_recursively(
		st, {}, {}, statements, symbols, blocks)
	.map_result<lang::spirit_item_filter>([&]() {
		return lang::spirit_item_filter{std::move(blocks)};
	});

	// This function is the point when we want to stop proceeding on any errors.
	// Otherwise the returned filter could contain broken state.
	if (has_errors(result.logs()))
		return std::move(result).logs();

	return result;
}

outcome<lang::item_filter>
compile_real_filter(
	settings st,
	const parser::ast::rf::ast_type& ast)
{
	lang::item_filter filter;
	filter.blocks.reserve(ast.size());

	log_container logs;

	for (const auto& block : ast) {
		[&]() -> outcome<lang::item_filter_block> {
			lang::item_filter_block filter_block;
			log_container logs;

			for (const auto& rule : block.rules) {
				rule.apply_visitor(x3::make_lambda_visitor<outcome<>>(
					[&](const parser::ast::rf::action& a) {
						return detail::real_filter_add_action(
							st, a, filter_block.actions);
					},
					[&](const parser::ast::rf::condition& c) {
						return detail::real_filter_add_condition(
							st, c, filter_block.conditions);
					}
				))
				.move_logs_to(logs);

				if (!should_continue(st.error_handling, logs))
					return logs;
			}

			filter_block.visibility = evaluate(block.visibility);
			return {std::move(filter_block), std::move(logs)};
		}()
		.map_result([&](lang::item_filter_block filter_block) {
			filter.blocks.push_back(std::move(filter_block));
		})
		.move_logs_to(logs);

		if (!should_continue(st.error_handling, logs))
			return logs;
	}

	return {std::move(filter), std::move(logs)};
}

}
