#include <fs/compiler/compiler.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/compiler/detail/conditions.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/item.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/monadic.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <optional>

namespace
{

using namespace fs;
using namespace fs::compiler;

using dmid = diagnostic_message_id;

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
[[nodiscard]] bool
add_constant_from_definition(
	settings st,
	const ast::sf::constant_definition& def,
	lang::symbol_table& symbols,
	diagnostics_container& diagnostics)
{
	const auto wanted_name_origin = parser::position_tag_of(def.value_name);
	const std::string& wanted_name = def.value_name.value.value;

	if (const auto it = symbols.find(wanted_name); it != symbols.end()) {
		const lang::position_tag place_of_original_name = parser::position_tag_of(it->second.name_origin);
		diagnostics.push_back(make_error(dmid::name_already_exists, wanted_name_origin, "name already exists"));
		diagnostics.push_back(make_note_first_defined_here(place_of_original_name));
		return false;
	}

	return detail::evaluate_value_expression(st, def.value, symbols, diagnostics)
		.map([&](lang::object obj) {
			const bool success = symbols.emplace(
				wanted_name,
				lang::named_object{std::move(obj), wanted_name_origin}).second;
			FS_ASSERT_MSG(success, "Insertion should succeed.");
			return success;
		})
		.value_or(false);
}

[[nodiscard]] bool
verify_string_condition_allows_value(
	lang::position_tag autogen_origin,
	const std::optional<lang::strings_condition>& opt_condition,
	std::string_view str,
	diagnostics_container& diagnostics)
{
	if (!opt_condition)
		return true;

	const auto& condition = *opt_condition;
	if (condition.find_match(str) != nullptr)
		return true;

	push_error_autogen_incompatible_condition(
		autogen_origin,
		condition.origin,
		std::string("\"").append(str).append("\""),
		diagnostics);
	return false;
}

std::string to_std_string(int value)
{
	return std::to_string(value);
}

std::string to_std_string(lang::rarity_type value)
{
	return std::string(lang::to_string_view(value));
}

template <typename T, typename U>
[[nodiscard]] bool
verify_range_condition_allows_value(
	lang::position_tag autogen_origin,
	lang::range_condition<T> condition,
	U value,
	diagnostics_container& diagnostics)
{
	auto result = true;

	if (condition.lower_bound && !condition.test_lower_bound(value)) {
		push_error_autogen_incompatible_condition(
			autogen_origin,
			(*condition.lower_bound).origin,
			to_std_string(value),
			diagnostics);
		result = false;
	}

	if (condition.upper_bound && !condition.test_upper_bound(value)) {
		push_error_autogen_incompatible_condition(
			autogen_origin,
			(*condition.upper_bound).origin,
			to_std_string(value),
			diagnostics);
		result = false;
	}

	return result;
}

[[nodiscard]] bool
verify_boolean_condition_allows_value(
	lang::position_tag autogen_origin,
	const std::optional<lang::boolean_condition>& opt_condition,
	bool value,
	diagnostics_container& diagnostics)
{
	if (!opt_condition)
		return true;

	const auto& condition = *opt_condition;
	if (condition.value.value == value)
		return true;

	push_error_autogen_incompatible_condition(
		autogen_origin,
		condition.origin,
		std::string(lang::to_string_view(value)),
		diagnostics);
	return false;
}

[[nodiscard]] bool
verify_integer_range_condition_exists(
	lang::position_tag visibility_origin,
	lang::position_tag autogen_origin,
	lang::integer_range_condition condition,
	std::string_view condition_keyword,
	diagnostics_container& diagnostics)
{
	if (condition.has_bound())
		return true;

	push_error_autogen_missing_condition(visibility_origin, autogen_origin, condition_keyword, diagnostics);
	return false;
}

[[nodiscard]] bool
verify_boolean_condition_exists(
	lang::position_tag visibility_origin,
	lang::position_tag autogen_origin,
	const std::optional<lang::boolean_condition>& opt_condition,
	std::string_view condition_keyword,
	diagnostics_container& diagnostics)
{
	if (opt_condition)
		return true;

	push_error_autogen_missing_condition(visibility_origin, autogen_origin, condition_keyword, diagnostics);
	return false;
}

[[nodiscard]] bool
verify_autogen_singular(
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	std::string_view class_name,
	diagnostics_container& diagnostics)
{
	auto result = true;

	if (!verify_string_condition_allows_value(autogen_origin, conditions.class_, class_name, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.rarity, lang::item::sentinel_rarity, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.item_level, lang::item::sentinel_item_level, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.quality, lang::item::sentinel_quality, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.linked_sockets, 0, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.height, 1, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.width, 1, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.gem_level, lang::item::sentinel_gem_level, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.map_tier, lang::item::sentinel_map_tier, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.corrupted_mods, 0, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.enchantment_passive_num, 0, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_enchanted, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_identified, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_corrupted, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_mirrored, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_elder_item, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_shaper_item, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_fractured_item, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_synthesised_item, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_shaped_map, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_elder_map, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_blighted_map, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_replica, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_alternate_quality, false, diagnostics))
		result = false;

	return result;
}

[[nodiscard]] bool
verify_autogen_gems(
	lang::position_tag visibility_origin,
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	diagnostics_container& diagnostics)
{
	auto result = true;

	if (!verify_integer_range_condition_exists(
		visibility_origin, autogen_origin, conditions.gem_level, lang::keywords::rf::gem_level, diagnostics))
	{
		result = false;
	}

	if (!verify_integer_range_condition_exists(
		visibility_origin, autogen_origin, conditions.quality, lang::keywords::rf::quality, diagnostics))
	{
		result = false;
	}

	if (!verify_boolean_condition_exists(
		visibility_origin, autogen_origin, conditions.is_corrupted, lang::keywords::rf::corrupted, diagnostics))
	{
		result = false;
	}

	return result;
}

[[nodiscard]] bool
verify_autogen_bases(
	lang::position_tag visibility_origin,
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	diagnostics_container& diagnostics)
{
	auto result = true;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.rarity, lang::rarity_type::normal, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.rarity, lang::rarity_type::magic, diagnostics))
		result = false;

	if (!verify_range_condition_allows_value(autogen_origin, conditions.rarity, lang::rarity_type::rare, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_corrupted, false, diagnostics))
		result = false;

	if (!verify_boolean_condition_allows_value(autogen_origin, conditions.is_mirrored, false, diagnostics))
		result = false;

	if (!verify_integer_range_condition_exists(
		visibility_origin, autogen_origin, conditions.item_level, lang::keywords::rf::item_level, diagnostics))
	{
		result = false;
	}

	if (!conditions.has_influence) {
		push_error_autogen_missing_condition(visibility_origin, autogen_origin, lang::keywords::rf::has_influence, diagnostics);
		result = false;
	}
	else if (!(*conditions.has_influence).exact_match_required) {
		diagnostics.push_back(make_error(
			dmid::autogen_incompatible_condition,
			(*conditions.has_influence).origin,
			"autogen-incompatible condition: HasInflunce needs to have strict matching (\"==\")"));
		diagnostics.push_back(make_note_minor(autogen_origin, "autogeneration specified here"));
		result = false;
	}

	return result;
}

[[nodiscard]] bool
verify_autogen_uniques(
	lang::position_tag autogen_origin,
	const lang::condition_set& conditions,
	diagnostics_container& diagnostics)
{
	/*
	 * Uniques are pretty unique so we are not checking any conditions
	 * besides rarity - uniques may contain very unusual values within
	 * certain properties (corruption, enchant, links, sockets, mods, etc)
	 */
	return verify_range_condition_allows_value(autogen_origin, conditions.rarity, lang::rarity_type::unique, diagnostics);
}

[[nodiscard]] bool
verify_autogen(
	lang::autogen_condition autogen,
	const lang::condition_set& conditions,
	lang::position_tag visibility_origin,
	diagnostics_container& diagnostics)
{
	switch (autogen.category) {
		using cat_t = lang::item_category;
		namespace cn = lang::item_class_names;

		case cat_t::currency:
			return verify_autogen_singular(autogen.origin, conditions, cn::currency_stackable, diagnostics);
		case cat_t::fragments:
			return verify_autogen_singular(autogen.origin, conditions, cn::map_fragments,      diagnostics);
		case cat_t::delirium_orbs:
			return verify_autogen_singular(autogen.origin, conditions, cn::delirium_orbs,      diagnostics);
		case cat_t::oils:
			return verify_autogen_singular(autogen.origin, conditions, cn::oils,               diagnostics);
		case cat_t::incubators:
			return verify_autogen_singular(autogen.origin, conditions, cn::incubator,          diagnostics);
		case cat_t::scarabs:
			return verify_autogen_singular(autogen.origin, conditions, cn::scarabs,            diagnostics);
		case cat_t::fossils:
			return verify_autogen_singular(autogen.origin, conditions, cn::fossils,            diagnostics);
		case cat_t::resonators:
			return verify_autogen_singular(autogen.origin, conditions, cn::resonators,         diagnostics);
		case cat_t::essences:
			return verify_autogen_singular(autogen.origin, conditions, cn::essences,           diagnostics);
		case cat_t::cards:
			return verify_autogen_singular(autogen.origin, conditions, cn::divination_card,    diagnostics);
		case cat_t::prophecies:
			return verify_autogen_singular(autogen.origin, conditions, cn::prophecies,         diagnostics);
		case cat_t::vials:
			return verify_autogen_singular(autogen.origin, conditions, cn::vials,              diagnostics);

		case cat_t::gems:
			return verify_autogen_gems(visibility_origin, autogen.origin, conditions, diagnostics);

		case cat_t::bases:
			return verify_autogen_bases(visibility_origin, autogen.origin, conditions, diagnostics);

		case cat_t::uniques_eq_unambiguous:
		case cat_t::uniques_eq_ambiguous:
		case cat_t::uniques_flasks_unambiguous:
		case cat_t::uniques_flasks_ambiguous:
		case cat_t::uniques_jewels_unambiguous:
		case cat_t::uniques_jewels_ambiguous:
		case cat_t::uniques_maps_unambiguous:
		case cat_t::uniques_maps_ambiguous:
			return verify_autogen_uniques(autogen.origin, conditions, diagnostics);
	}

	// failed to cover given category - add internal error and return failure
	push_error_internal_compiler_error(__func__, autogen.origin, diagnostics);
	return false;
}

[[nodiscard]] lang::item_visibility
evaluate(parser::ast::common::static_visibility_statement visibility)
{
	return lang::item_visibility{
		visibility.show ? lang::item_visibility_policy::show : lang::item_visibility_policy::hide,
		parser::position_tag_of(visibility)
	};
}

[[nodiscard]] boost::optional<lang::item_visibility>
evaluate(
	settings st,
	parser::ast::sf::dynamic_visibility_statement visibility,
	const lang::symbol_table& symbols,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, visibility.seq, symbols, 1, 1, diagnostics)
		.flat_map([&](lang::object obj) {
			FS_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::boolean>(obj.values[0], diagnostics);
		})
		.map([&](lang::boolean b) {
			const lang::position_tag origin = parser::position_tag_of(visibility);
			if (b.value) {
				return lang::item_visibility{lang::item_visibility_policy::show, origin};
			}
			else {
				if (visibility.policy.discard)
					return lang::item_visibility{lang::item_visibility_policy::discard, origin};
				else
					return lang::item_visibility{lang::item_visibility_policy::hide, origin};
			}
		});
}

[[nodiscard]] boost::optional<lang::item_visibility>
evaluate(
	settings st,
	parser::ast::sf::visibility_statement visibility,
	const lang::symbol_table& symbols,
	diagnostics_container& diagnostics)
{
	return visibility.apply_visitor(x3::make_lambda_visitor<boost::optional<lang::item_visibility>>(
		[](parser::ast::common::static_visibility_statement visibility) { return evaluate(visibility); },
		[&](parser::ast::sf::dynamic_visibility_statement visibility) { return evaluate(st, visibility, symbols, diagnostics); }
	));
}

[[nodiscard]] boost::optional<lang::spirit_item_filter_block>
make_spirit_filter_block(
	settings /* st */,
	lang::item_visibility visibility,
	lang::spirit_condition_set conditions,
	lang::action_set actions,
	lang::block_continuation continuation,
	diagnostics_container& diagnostics)
{
	if (conditions.price.has_bound() && !conditions.autogen.has_value()) {
		std::optional<lang::position_tag> price_first_origin = conditions.price.first_origin();
		FS_ASSERT_MSG(
			price_first_origin.has_value(),
			"Price condition has a bound. It must have at least 1 origin.");
		diagnostics.push_back(make_error(
			dmid::price_without_autogen,
			visibility.origin,
			"generation of a block with price bound has missing autogeneration specifier"));
		diagnostics.push_back(make_note_minor(*price_first_origin, "price bound specified here"));
		if (auto price_second_origin = conditions.price.second_origin(); price_second_origin.has_value()) {
			diagnostics.push_back(make_note_minor(*price_second_origin, "another price bound specified here"));
		}
		return boost::none;
	}

	std::optional<lang::autogen_extension> autogen;
	if (conditions.autogen) {
		if (verify_autogen(*conditions.autogen, conditions.conditions, visibility.origin, diagnostics)) {
			autogen = lang::autogen_extension{conditions.price, *conditions.autogen};
		}
		else {
			return boost::none;
		}
	}

	return lang::spirit_item_filter_block{
		lang::item_filter_block(
			visibility,
			std::move(conditions.conditions),
			std::move(actions),
			continuation
		),
		autogen};
}

lang::block_continuation
to_block_continuation(
	boost::optional<ast::common::continue_statement> statement)
{
	if (statement)
		return lang::block_continuation{parser::position_tag_of(*statement)};
	else
		return lang::block_continuation{std::nullopt};
}

bool
compile_statements_recursively(
	settings st,
	lang::spirit_condition_set parent_conditions,
	lang::action_set parent_actions,
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols,
	std::vector<lang::spirit_item_filter_block>& blocks,
	diagnostics_container& diagnostics)
{
	for (const ast::sf::statement& statement : statements) {
		bool result = statement.apply_visitor(x3::make_lambda_visitor<bool>(
			[&](const ast::sf::action& action) {
				return detail::spirit_filter_add_action(st, action, symbols, parent_actions, diagnostics);
			},
			[&](const ast::sf::behavior_statement& bs) {
				boost::optional<lang::item_visibility> visibility = evaluate(st, bs.visibility, symbols, diagnostics);
				if (!visibility)
					return false;

				boost::optional<lang::spirit_item_filter_block> block = make_spirit_filter_block(
					st,
					*visibility,
					parent_conditions,
					parent_actions,
					to_block_continuation(bs.continue_),
					diagnostics);

				if (block) {
					blocks.push_back(std::move(*block));
					return true;
				}
				else {
					return false;
				}
			},
			[&](const ast::sf::rule_block& nested_block) {
				// explicitly make a copy of parent conditions - the call stack will preserve
				// old instance while nested blocks can add additional conditions that have limited lifetime
				lang::spirit_condition_set nested_conditions(parent_conditions);

				if (detail::spirit_filter_add_conditions(
					st, nested_block.conditions, symbols, nested_conditions, diagnostics))
				{
					return compile_statements_recursively(
						st,
						std::move(nested_conditions),
						parent_actions,
						nested_block.statements,
						symbols,
						blocks,
						diagnostics);
				}
				else {
					return false;
				}
			}));

		if (!result && st.error_handling.stop_on_error)
			return false;
	}

	return true;
}

} // namespace

namespace fs::compiler
{

boost::optional<lang::symbol_table>
resolve_spirit_filter_symbols(
	settings st,
	const std::vector<parser::ast::sf::definition>& definitions,
	diagnostics_container& diagnostics)
{
	lang::symbol_table symbols;

	for (const auto& def : definitions) {
		const bool result = add_constant_from_definition(st, def.def, symbols, diagnostics);

		if (!result && st.error_handling.stop_on_error)
			return boost::none;
	}

	return symbols;
}

boost::optional<lang::spirit_item_filter>
compile_spirit_filter_statements(
	settings st,
	const std::vector<ast::sf::statement>& statements,
	const lang::symbol_table& symbols,
	diagnostics_container& diagnostics)
{
	std::vector<lang::spirit_item_filter_block> blocks;
	// start with empty conditions and actions
	// thats the default state before any nesting
	if (!compile_statements_recursively(st, {}, {}, statements, symbols, blocks, diagnostics))
		return boost::none;

	// This function is the point when we want to stop proceeding on any errors.
	// Otherwise the returned filter could contain broken state.
	if (has_errors(diagnostics))
		return boost::none;

	return lang::spirit_item_filter{std::move(blocks)};
}

boost::optional<lang::item_filter>
compile_real_filter(
	settings st,
	const parser::ast::rf::ast_type& ast,
	diagnostics_container& diagnostics)
{
	lang::item_filter filter;
	filter.blocks.reserve(ast.size());

	for (const auto& block : ast) {
		auto filter_block = [&]() -> boost::optional<lang::item_filter_block> {
			lang::item_filter_block filter_block(evaluate(block.visibility));

			for (const auto& rule : block.rules) {
				const bool result = rule.apply_visitor(x3::make_lambda_visitor<bool>(
					[&](const parser::ast::rf::action& a) {
						return detail::real_filter_add_action(
							st, a, filter_block.actions, diagnostics);
					},
					[&](const parser::ast::rf::condition& c) {
						return detail::real_filter_add_condition(
							st, c, filter_block.conditions, diagnostics);
					}
				));

				if (!result && st.error_handling.stop_on_error)
					return boost::none;
			}

			return filter_block;
		}();

		if (filter_block)
			filter.blocks.push_back(std::move(*filter_block));
		else if (st.error_handling.stop_on_error)
			return boost::none;
	}

	return filter;
}

}
