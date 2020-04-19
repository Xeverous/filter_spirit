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
[[nodiscard]] std::optional<compile_error>
add_constant_from_definition(
	const ast::sf::constant_definition& def,
	lang::symbol_table& symbols)
{
	const auto wanted_name_origin = parser::position_tag_of(def.value_name);
	const std::string& wanted_name = def.value_name.value.value;

	if (const auto it = symbols.find(wanted_name); it != symbols.end()) {
		const lang::position_tag place_of_original_name = parser::position_tag_of(it->second.name_origin);
		return errors::name_already_exists{wanted_name_origin, place_of_original_name};
	}

	std::variant<lang::object, compile_error> expr_result =
		detail::evaluate_value_expression(def.value, symbols);

	if (std::holds_alternative<compile_error>(expr_result))
		return std::get<compile_error>(std::move(expr_result));

	const bool success = symbols.emplace(
		wanted_name,
		lang::named_object{
			std::get<lang::object>(std::move(expr_result)),
			wanted_name_origin}).second;
	BOOST_ASSERT_MSG(success, "Insertion should succeed.");
	(void) success; // shut unused variable warning
	return std::nullopt;
}

// ---- compile_spirit_filter_statements helpers ----

bool condition_contains(const lang::strings_condition& condition, std::string_view fragment)
{
	for (const auto& str : condition.strings) {
		if (utility::contains(str, fragment))
			return true;
	}

	return false;
}

[[nodiscard]] std::optional<compile_error>
verify_autogen_cards(
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	if (conditions.class_.has_value()) {
		if (!condition_contains(*conditions.class_, fs::lang::classes::cards)) {
			return errors::autogen_error{
				errors::autogen_error_cause::invalid_class_condition,
				autogen_origin,
				(*conditions.class_).origin,
				visibility_origin
			};
		}
	}

	if (conditions.prophecy.has_value()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen_origin,
			(*conditions.prophecy).origin,
			visibility_origin};
	}

	if (conditions.gem_level.has_bound()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen_origin,
			*conditions.gem_level.first_origin(),
			visibility_origin};
	}

	if (conditions.map_tier.has_bound()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen_origin,
			*conditions.map_tier.first_origin(),
			visibility_origin};
	}

	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
verify_autogen_currency(
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	if (conditions.class_.has_value()) {
		if (!condition_contains(*conditions.class_, "Currency")) {
			return errors::autogen_error{
				errors::autogen_error_cause::invalid_class_condition,
				autogen_origin,
				(*conditions.class_).origin,
				visibility_origin
			};
		}
	}

	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
verify_autogen_uniques(
	lang::autogen_condition autogen,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	if (!conditions.rarity.includes(lang::rarity_type::unique)) {
		return errors::autogen_error{
			errors::autogen_error_cause::invalid_rarity_condition,
			autogen.origin,
			*conditions.rarity.first_origin(),
			visibility_origin
		};
	}

	if (conditions.stack_size.has_bound()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			*conditions.stack_size.first_origin(),
			visibility_origin
		};
	}

	if (conditions.gem_level.has_bound()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			*conditions.gem_level.first_origin(),
			visibility_origin
		};
	}

	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
verify_autogen(
	lang::autogen_condition autogen,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin)
{
	// all autogens require empty BaseType
	if (conditions.base_type.has_value()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			(*conditions.base_type).origin,
			visibility_origin};
	}

	using cat_t = lang::item_category;

	// all autogens except prophecies require empty Prophecy
	if (autogen.category != cat_t::prophecies && conditions.prophecy.has_value()) {
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			(*conditions.prophecy).origin,
			visibility_origin};
	}

	// all autogens except uniques_maps_* require empty MapTier
	if (const bool is_map_autogen =
			autogen.category == cat_t::uniques_maps_unambiguous ||
			autogen.category == cat_t::uniques_maps_ambiguous;
		is_map_autogen && conditions.map_tier.has_bound())
	{
		return errors::autogen_error{
			errors::autogen_error_cause::expected_empty_condition,
			autogen.origin,
			*conditions.map_tier.first_origin(),
			visibility_origin};
	}

	switch (autogen.category) {
		case cat_t::cards:
			return verify_autogen_cards(autogen.origin, conditions, visibility_origin);

		case cat_t::scarabs:
		case cat_t::incubators:
			return std::nullopt; // TODO add checking

		case cat_t::prophecies:
		case cat_t::essences:
		case cat_t::fossils:
		case cat_t::resonators:
		case cat_t::oils:
		case cat_t::catalysts:
			return verify_autogen_currency(autogen.origin, conditions, visibility_origin);

		case cat_t::uniques_eq_unambiguous:
		case cat_t::uniques_eq_ambiguous:
		case cat_t::uniques_flasks_unambiguous:
		case cat_t::uniques_flasks_ambiguous:
		case cat_t::uniques_jewels_unambiguous:
		case cat_t::uniques_jewels_ambiguous:
		case cat_t::uniques_maps_unambiguous:
		case cat_t::uniques_maps_ambiguous:
			return verify_autogen_uniques(autogen, conditions, visibility_origin);
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::spirit_filter_verify_autogen_conditions,
		autogen.origin
	};
}

[[nodiscard]] std::variant<lang::spirit_item_filter_block, compile_error>
make_spirit_filter_block(
	lang::item_visibility visibility,
	lang::spirit_condition_set conditions,
	lang::action_set actions)
{
	if (conditions.price.has_bound() && !conditions.autogen.has_value()) {
		std::optional<lang::position_tag> price_first_origin = conditions.price.first_origin();
		BOOST_ASSERT_MSG(
			price_first_origin.has_value(),
			"Price condition has a bound. It must have at least 1 origin.");
		return errors::price_without_autogen{
			visibility.origin,
			*price_first_origin,
			conditions.price.second_origin()};
	}

	std::optional<lang::autogen_extension> autogen_ext;
	if (conditions.autogen.has_value()) {
		std::optional<compile_error> err = verify_autogen(
			*conditions.autogen, conditions.conditions, visibility.origin);

		if (err)
			return *std::move(err);

		autogen_ext = lang::autogen_extension{conditions.price, *conditions.autogen};
	}

	return lang::spirit_item_filter_block{
		lang::item_filter_block{
			visibility,
			std::move(conditions.conditions),
			std::move(actions)
		},
		autogen_ext
	};
}

std::optional<compile_error> compile_statements_recursively(
	lang::spirit_condition_set parent_conditions,
	lang::action_set parent_actions,
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols,
	std::vector<lang::spirit_item_filter_block>& blocks)
{
	for (const ast::sf::statement& statement : statements) {
		using result_type = std::optional<compile_error>;

		auto error = statement.apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::action& action) {
				return detail::spirit_filter_add_action(action, symbols, parent_actions);
			},
			[&](const ast::sf::visibility_statement& vs) -> result_type {
				std::variant<lang::spirit_item_filter_block, compile_error> blk_or_err =
					make_spirit_filter_block(
						lang::item_visibility{vs.show, parser::position_tag_of(vs)},
						parent_conditions,
						parent_actions);

				if (std::holds_alternative<compile_error>(blk_or_err)) {
					return std::get<compile_error>(std::move(blk_or_err));
				}

				blocks.push_back(std::get<lang::spirit_item_filter_block>(std::move(blk_or_err)));
				return std::nullopt;
			},
			[&](const ast::sf::rule_block& nested_block) -> result_type {
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::spirit_condition_set nested_conditions(parent_conditions);
				std::optional<compile_error> error = detail::spirit_filter_add_conditions(
					nested_block.conditions, symbols, nested_conditions);
				if (error)
					return *std::move(error);

				return compile_statements_recursively(
					std::move(nested_conditions),
					parent_actions,
					nested_block.statements,
					symbols,
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
resolve_spirit_filter_symbols(const std::vector<parser::ast::sf::definition>& definitions)
{
	lang::symbol_table symbols;

	for (const auto& def : definitions) {
		std::optional<compile_error> error = add_constant_from_definition(def.def, symbols);

		if (error)
			return *std::move(error);
	}

	return symbols;
}

std::variant<lang::spirit_item_filter, compile_error>
compile_spirit_filter_statements(
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols)
{
	std::vector<lang::spirit_item_filter_block> blocks;
	// start with empty conditions and actions
	// thats the default state before any nesting
	std::optional<compile_error> err = compile_statements_recursively(
		{}, {}, statements, symbols, blocks);
	if (err) {
		return *std::move(err);
	}

	return lang::spirit_item_filter{std::move(blocks)};
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

		filter_block.visibility = lang::item_visibility{
			block.visibility.show,
			parser::position_tag_of(block.visibility)
		};
		filter.blocks.push_back(std::move(filter_block));
	}

	return filter;
}

}
