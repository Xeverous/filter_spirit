#include <fs/compiler/detail/conditions.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/condition_set.hpp>
#include <fs/lang/limits.hpp>
#include <fs/utility/assert.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <boost/optional.hpp>

#include <utility>

namespace ast = fs::parser::ast;

namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

using str_vec_t = std::vector<lang::string>;
using dmid = diagnostic_message_id;

// ---- generic helpers ----

template <typename T>
[[nodiscard]] bool
add_range_condition(
	lang::comparison_type comparison_type,
	T value,
	lang::position_tag condition_origin,
	lang::range_condition<T>& target,
	diagnostics_container& diagnostics)
{
	switch (comparison_type) {
		case lang::comparison_type::equal_soft:
		case lang::comparison_type::equal_hard: {
			if (target.lower_bound.has_value()) {
				push_error_lower_bound_redefinition(condition_origin, (*target.lower_bound).origin, diagnostics);
				return false;
			}

			if (target.upper_bound.has_value()) {
				push_error_upper_bound_redefinition(condition_origin, (*target.upper_bound).origin, diagnostics);
				return false;
			}

			target.set_exact(value, condition_origin);
			return true;
		}
		case lang::comparison_type::less: {
			if (target.upper_bound.has_value()) {
				push_error_upper_bound_redefinition(condition_origin, (*target.upper_bound).origin, diagnostics);
				return false;
			}

			target.set_upper_bound(value, false, condition_origin);
			return true;
		}
		case lang::comparison_type::less_equal: {
			if (target.upper_bound.has_value()) {
				push_error_upper_bound_redefinition(condition_origin, (*target.upper_bound).origin, diagnostics);
				return false;
			}

			target.set_upper_bound(value, true, condition_origin);
			return true;
		}
		case lang::comparison_type::greater: {
			if (target.lower_bound.has_value()) {
				push_error_lower_bound_redefinition(condition_origin, (*target.lower_bound).origin, diagnostics);
				return false;
			}

			target.set_lower_bound(value, false, condition_origin);
			return true;
		}
		case lang::comparison_type::greater_equal: {
			if (target.lower_bound.has_value()) {
				push_error_lower_bound_redefinition(condition_origin, (*target.lower_bound).origin, diagnostics);
				return false;
			}

			target.set_lower_bound(value, true, condition_origin);
			return true;
		}
	}

	push_error_internal_compiler_error(__func__, condition_origin, diagnostics);
	return false;
}

[[nodiscard]] bool
add_numeric_comparison_condition(
	lang::numeric_comparison_condition_property property,
	lang::comparison_type cmp,
	lang::integer intgr,
	lang::position_tag condition_origin,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	switch (property) {
		case lang::numeric_comparison_condition_property::item_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.item_level, diagnostics);
		}
		case lang::numeric_comparison_condition_property::drop_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.drop_level, diagnostics);
		}
		case lang::numeric_comparison_condition_property::quality: {
			return add_range_condition(cmp, intgr, condition_origin, set.quality, diagnostics);
		}
		case lang::numeric_comparison_condition_property::linked_sockets: {
			return add_range_condition(cmp, intgr, condition_origin, set.linked_sockets, diagnostics);
		}
		case lang::numeric_comparison_condition_property::height: {
			return add_range_condition(cmp, intgr, condition_origin, set.height, diagnostics);
		}
		case lang::numeric_comparison_condition_property::width: {
			return add_range_condition(cmp, intgr, condition_origin, set.width, diagnostics);
		}
		case lang::numeric_comparison_condition_property::stack_size: {
			return add_range_condition(cmp, intgr, condition_origin, set.stack_size, diagnostics);
		}
		case lang::numeric_comparison_condition_property::gem_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.gem_level, diagnostics);
		}
		case lang::numeric_comparison_condition_property::map_tier: {
			return add_range_condition(cmp, intgr, condition_origin, set.map_tier, diagnostics);
		}
		case lang::numeric_comparison_condition_property::area_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.area_level, diagnostics);
		}
		case lang::numeric_comparison_condition_property::corrupted_mods: {
			return add_range_condition(cmp, intgr, condition_origin, set.corrupted_mods, diagnostics);
		}
		case lang::numeric_comparison_condition_property::enchantment_passive_num: {
			return add_range_condition(cmp, intgr, condition_origin, set.enchantment_passive_num, diagnostics);
		}
		case lang::numeric_comparison_condition_property::base_defence_percentile: {
			return add_range_condition(cmp, intgr, condition_origin, set.base_defence_percentile, diagnostics);
		}
		case lang::numeric_comparison_condition_property::base_armour: {
			return add_range_condition(cmp, intgr, condition_origin, set.base_armour, diagnostics);
		}
		case lang::numeric_comparison_condition_property::base_evasion: {
			return add_range_condition(cmp, intgr, condition_origin, set.base_evasion, diagnostics);
		}
		case lang::numeric_comparison_condition_property::base_energy_shield: {
			return add_range_condition(cmp, intgr, condition_origin, set.base_energy_shield, diagnostics);
		}
		case lang::numeric_comparison_condition_property::base_ward: {
			return add_range_condition(cmp, intgr, condition_origin, set.base_ward, diagnostics);
		}
		case lang::numeric_comparison_condition_property::has_searing_exarch_implicit: {
			return add_range_condition(cmp, intgr, condition_origin, set.has_searing_exarch_implicit, diagnostics);
		}
		case lang::numeric_comparison_condition_property::has_eater_of_worlds_implicit: {
			return add_range_condition(cmp, intgr, condition_origin, set.has_eater_of_worlds_implicit, diagnostics);
		}
	}

	push_error_internal_compiler_error(__func__, condition_origin, diagnostics);
	return false;
}

template <typename Condition>
[[nodiscard]] bool
add_non_range_condition(
	Condition condition,
	std::optional<Condition>& target,
	diagnostics_container& diagnostics)
{
	if (target.has_value()) {
		diagnostics.push_back(make_error(
			dmid::condition_redefinition,
			condition.origin,
			"condition redefinition (the same condition can not be specified again in the same block or nested blocks)"));
		diagnostics.push_back(make_note_first_defined_here((*target).origin));
		return false;
	}

	target = std::move(condition);
	return true;
}

[[nodiscard]] bool
add_ranged_strings_condition(
	lang::strings_condition strings_cond,
	lang::comparison_type cmp,
	boost::optional<lang::integer> intgr,
	std::optional<lang::ranged_strings_condition>& target,
	diagnostics_container& diagnostics)
{
	lang::integer_range_condition integer_cond;
	if (intgr.has_value()) {
		if (!add_range_condition(cmp, *intgr, strings_cond.origin, integer_cond, diagnostics))
			return false;
	}

	return add_non_range_condition(
		lang::ranged_strings_condition{
			integer_cond,
			std::move(strings_cond),
			strings_cond.origin
		},
		target,
		diagnostics);
}

[[nodiscard]] bool
add_string_array_condition(
	lang::string_array_condition_property property,
	str_vec_t strings,
	bool is_exact_match,
	lang::position_tag condition_origin,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	switch (property) {
		case lang::string_array_condition_property::class_: {
			return add_non_range_condition(
				lang::strings_condition{std::move(strings), is_exact_match, condition_origin},
				set.class_,
				diagnostics);
		}
		case lang::string_array_condition_property::base_type: {
			return add_non_range_condition(
				lang::strings_condition{std::move(strings), is_exact_match, condition_origin},
				set.base_type,
				diagnostics);
		}
		case lang::string_array_condition_property::enchantment_passive_node: {
			return add_non_range_condition(
				lang::strings_condition{std::move(strings), is_exact_match, condition_origin},
				set.enchantment_passive_node,
				diagnostics);
		}
		case lang::string_array_condition_property::archnemesis_mod: {
			return add_non_range_condition(
				lang::strings_condition{std::move(strings), is_exact_match, condition_origin},
				set.archnemesis_mod,
				diagnostics);
		}
	}

	push_error_internal_compiler_error(__func__, condition_origin, diagnostics);
	return false;
}

[[nodiscard]] bool
add_ranged_string_array_condition(
	lang::ranged_string_array_condition_property property,
	str_vec_t strings,
	lang::comparison_type comparison,
	lang::position_tag comparison_origin,
	boost::optional<lang::integer> integer,
	lang::position_tag condition_origin,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	auto push_error_invalid_ranged_strings_condition = [&](boost::optional<lang::position_tag> integer_origin) {
		diagnostics.push_back(make_error(
			dmid::invalid_ranged_strings_condition,
			condition_origin,
			"invalid ranged strings array condition"));
		if (lang::is_valid(comparison_origin)) {
			diagnostics.push_back(make_note(
				dmid::minor_note,
				comparison_origin,
				"only == is allowed to be without a number"));
		}
		if (integer_origin) {
			diagnostics.push_back(make_note(
				dmid::minor_note,
				*integer_origin,
				"if an integer is present, it must be preceded by a comparison operator"));
		}
	};
	/*
	 * Error checking:
	 *
	 * (op means any other operator than ==)
	 * (N means integer literal)
	 *
	 * <ConditionKeyword>     "..." #     valid
	 * <ConditionKeyword> ==  "..." #     valid
	 * <ConditionKeyword> op  "..." # NOT valid
	 * <ConditionKeyword>   N "..." # NOT valid
	 * <ConditionKeyword> ==N "..." #     valid
	 * <ConditionKeyword> opN "..." #     valid
	 */
	if (comparison != lang::comparison_type::equal_soft && comparison != lang::comparison_type::equal_hard && !integer) {
		push_error_invalid_ranged_strings_condition(boost::none);
		return false;
	}
	else if (comparison == lang::comparison_type::equal_soft && integer.has_value()) {
		push_error_invalid_ranged_strings_condition((*integer).origin);
		return false;
	}

	const bool is_exact_match = comparison == lang::comparison_type::equal_hard;
	switch (property) {
		case lang::ranged_string_array_condition_property::has_explicit_mod: {
			return add_ranged_strings_condition(
				lang::strings_condition{std::move(strings), is_exact_match, condition_origin},
				comparison,
				integer,
				set.has_explicit_mod,
				diagnostics);
		}
		case lang::ranged_string_array_condition_property::has_enchantment: {
			return add_ranged_strings_condition(
				lang::strings_condition{std::move(strings), is_exact_match, condition_origin},
				comparison,
				integer,
				set.has_enchantment,
				diagnostics);
		}
	}

	push_error_internal_compiler_error(__func__, condition_origin, diagnostics);
	return false;
}

/*
 * Empty container represents "HasInfluence None".
 *
 * This is a bit limiting implementation, because
 * it does not allow to track all possible origins
 * (especially "None" literal origin) but such detailed
 * debug information is not currently needed; also many
 * other conditions also track only condition name origin.
*/
using influences_container = boost::container::static_vector<
	lang::influence,
	lang::limits::max_filter_influences
>;

[[nodiscard]] bool
add_has_influence_condition(
	settings st,
	influences_container influences,
	bool is_exact_match,
	lang::position_tag origin,
	std::optional<lang::influences_condition>& target,
	diagnostics_container& diagnostics)
{
	for (std::size_t i = 0; i < influences.size(); ++i) {
		for (std::size_t j = 0; j < influences.size(); ++j) {
			if (j != i && influences[i] == influences[j]) {
				diagnostics.push_back(make_error(dmid::duplicate_influence, influences[j].origin, "duplicate influence"));
				diagnostics.push_back(make_note_first_defined_here(influences[i].origin));

				if (st.error_handling.stop_on_error)
					return false;
			}
		}
	}

	const auto contains = [&](lang::influence_type type) {
		for (const auto infl : influences)
			if (infl.value == type)
				return true;

		return false;
	};

	return add_non_range_condition(
		lang::influences_condition{
			lang::influence_info{
				contains(lang::influence_type::shaper),
				contains(lang::influence_type::elder),
				contains(lang::influence_type::crusader),
				contains(lang::influence_type::redeemer),
				contains(lang::influence_type::hunter),
				contains(lang::influence_type::warlord)
			},
			is_exact_match,
			origin
		},
		target,
		diagnostics);
}

[[nodiscard]] bool
add_socket_spec_condition(
	bool links_matter,
	lang::socket_spec_condition condition,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	auto& target = links_matter ? set.socket_group : set.sockets;
	return add_non_range_condition(condition, target, diagnostics);
}

[[nodiscard]] bool
add_boolean_condition(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	lang::boolean_condition_property property,
	lang::condition_set& condition_set,
	diagnostics_container& diagnostics)
{
	switch (property) {
		case lang::boolean_condition_property::identified: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_identified,
				diagnostics);
		}
		case lang::boolean_condition_property::corrupted: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_corrupted,
				diagnostics);
		}
		case lang::boolean_condition_property::mirrored: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_mirrored,
				diagnostics);
		}
		case lang::boolean_condition_property::elder_item: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_elder_item,
				diagnostics);
		}
		case lang::boolean_condition_property::shaper_item: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_shaper_item,
				diagnostics);
		}
		case lang::boolean_condition_property::fractured_item: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_fractured_item,
				diagnostics);
		}
		case lang::boolean_condition_property::synthesised_item: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_synthesised_item,
				diagnostics);
		}
		case lang::boolean_condition_property::any_enchantment: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_enchanted,
				diagnostics);
		}
		case lang::boolean_condition_property::shaped_map: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_shaped_map,
				diagnostics);
		}
		case lang::boolean_condition_property::elder_map: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_elder_map,
				diagnostics);
		}
		case lang::boolean_condition_property::blighted_map: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_blighted_map,
				diagnostics);
		}
		case lang::boolean_condition_property::replica: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_replica,
				diagnostics);
		}
		case lang::boolean_condition_property::alternate_quality: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_alternate_quality,
				diagnostics);
		}
		case lang::boolean_condition_property::scourged: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_scourged,
				diagnostics);
		}
		case lang::boolean_condition_property::uber_blighted_map: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.is_uber_blighted_map,
				diagnostics);
		}
		case lang::boolean_condition_property::has_implicit_mod: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.has_implicit_mod,
				diagnostics);
		}
		case lang::boolean_condition_property::has_crucible_passive_tree: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.has_crucible_passive_tree,
				diagnostics);
		}
		case lang::boolean_condition_property::transfigured_gem: {
			return add_non_range_condition(
				lang::boolean_condition{boolean, condition_origin},
				condition_set.transfigured_gem,
				diagnostics);
		}
	}

	push_error_internal_compiler_error(__func__, condition_origin, diagnostics);
	return false;
}

// ---- spirit filter helpers ----

[[nodiscard]] bool
spirit_filter_add_price_comparison_condition(
	settings st,
	const ast::sf::price_comparison_condition& condition,
	const symbol_table& symbols,
	lang::spirit_condition_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1, diagnostics)
		.flat_map([&](lang::object obj) {
			FS_ASSERT(obj.values.size() == 1u);
			return detail::get_as_fractional(obj.values[0], diagnostics);
		})
		.map([&](lang::fractional frac) {
			return add_range_condition(
				condition.comparison_type.value,
				frac,
				parser::position_tag_of(condition),
				set.price,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_rarity_comparison_condition(
	settings st,
	const ast::sf::rarity_comparison_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1, diagnostics)
		.flat_map([&](lang::object obj) {
			FS_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::rarity>(obj.values[0], diagnostics);
		})
		.map([&](lang::rarity rar) {
			return add_range_condition(
				condition.comparison_type.value,
				rar,
				parser::position_tag_of(condition),
				set.rarity,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_numeric_comparison_condition(
	settings st,
	const ast::sf::numeric_comparison_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1, diagnostics)
		.flat_map([&](lang::object obj) {
			FS_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::integer>(obj.values[0], diagnostics);
		})
		.map([&](lang::integer intgr) {
			return add_numeric_comparison_condition(
				condition.property,
				condition.comparison_type.value,
				intgr,
				parser::position_tag_of(condition),
				set,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] boost::optional<str_vec_t>
spirit_filter_make_string_array(
	settings st,
	const lang::object& obj,
	diagnostics_container& diagnostics)
{
	str_vec_t strings;

	for (auto& sobj : obj.values) {
		// skip "None"s - they may come from variables
		// if an object is not none - ignore the problem and move forward
		// because of ignoring all logs are thrown away
		diagnostics_container diagnostics_none;
		if (detail::get_as<lang::none>(sobj, diagnostics_none).has_value())
			continue;

		boost::optional<lang::string> str = detail::get_as<lang::string>(sobj, diagnostics);

		if (str)
			strings.push_back(std::move(*str));
		else if (st.error_handling.stop_on_error)
			return boost::none;
	}

	return strings;
}

[[nodiscard]] bool
spirit_filter_add_string_array_condition(
	settings st,
	const ast::sf::string_array_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, boost::none, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<str_vec_t> {
			return spirit_filter_make_string_array(st, obj, diagnostics);
		})
		.map([&](str_vec_t strings) {
			return add_string_array_condition(
				condition.property,
				std::move(strings),
				condition.exact_match.required,
				parser::position_tag_of(condition),
				set,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_ranged_string_array_condition(
	settings st,
	const ast::sf::ranged_string_array_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, boost::none, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<str_vec_t> {
			return spirit_filter_make_string_array(st, obj, diagnostics);
		})
		.map([&](str_vec_t strings) {
			return add_ranged_string_array_condition(
				condition.property,
				std::move(strings),
				condition.comparison_type.value,
				parser::position_tag_of(condition.comparison_type),
				condition.integer.map([](ast::common::integer_literal il) { return detail::evaluate(il); }),
				parser::position_tag_of(condition),
				set,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_has_influence_condition(
	settings st,
	const ast::sf::has_influence_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, lang::limits::max_filter_influences, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<influences_container> {
			if (obj.values.size() == 1u) {
				diagnostics_container diagnostics_none; // ignore logs and move on if get_as<none> fails
				if (detail::get_as<lang::none>(obj.values.front(), diagnostics_none).has_value())
					return influences_container();
			}

			influences_container influences;

			for (auto& sobj : obj.values) {
				boost::optional<lang::influence> infl = detail::get_as<lang::influence>(sobj, diagnostics);

				if (infl)
					influences.emplace_back(*infl);
				else if (st.error_handling.stop_on_error)
					return boost::none;
			}

			return influences;
		})
		.map([&](influences_container influences) {
			return add_has_influence_condition(
				st,
				std::move(influences),
				condition.exact_match.required,
				parser::position_tag_of(condition),
				set.has_influence,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_socket_spec_condition(
	settings st,
	const ast::sf::socket_spec_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	using specs_t = lang::socket_spec_condition::container_type;
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, boost::none, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<specs_t> {
			specs_t specs;

			for (auto& val : obj.values) {
				boost::optional<lang::socket_spec> ss = detail::get_as_socket_spec(val, diagnostics);

				if (ss)
					specs.push_back(*ss);
				else if (st.error_handling.stop_on_error)
					return boost::none;
			}

			return specs;
		})
		.map([&](specs_t specs) {
			return add_socket_spec_condition(
				condition.links_matter,
				lang::socket_spec_condition{
					condition.comparison_type.value,
					std::move(specs),
					parser::position_tag_of(condition)
				},
				set,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_boolean_condition(
	settings st,
	const ast::sf::boolean_condition& condition,
	const symbol_table& symbols,
	lang::condition_set& condition_set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1, diagnostics)
		.flat_map([&](lang::object obj) {
			FS_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::boolean>(obj.values[0], diagnostics);
		})
		.map([&](lang::boolean b) {
			return add_boolean_condition(
				b,
				parser::position_tag_of(condition),
				condition.property,
				condition_set,
				diagnostics);
		})
		.value_or(false);
}

// ---- real filter helpers ----

[[nodiscard]] bool
real_filter_add_numeric_comparison_condition(
	ast::rf::numeric_condition numeric_condition,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	const lang::integer intgr = detail::evaluate(numeric_condition.integer);
	const auto origin = parser::position_tag_of(numeric_condition);
	const lang::comparison_type cmp = numeric_condition.comparison_type.value;
	return add_numeric_comparison_condition(numeric_condition.property, cmp, intgr, origin, set, diagnostics);
}

[[nodiscard]] str_vec_t
real_filter_make_string_array(
	const parser::ast::rf::string_array& strings)
{
	str_vec_t result;
	result.reserve(strings.size());
	for (const auto& str : strings)
		result.push_back(detail::evaluate(str));
	return result;
}

[[nodiscard]] bool
real_filter_add_string_array_condition(
	const parser::ast::rf::string_array_condition& condition,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	str_vec_t strings = real_filter_make_string_array(condition.strings);
	const bool exact_match = condition.exact_match.required;
	const auto origin = parser::position_tag_of(condition);
	return add_string_array_condition(condition.property, std::move(strings), exact_match, origin, set, diagnostics);
}

[[nodiscard]] bool
real_filter_add_ranged_string_array_condition(
	const parser::ast::rf::ranged_string_array_condition& condition,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	return add_ranged_string_array_condition(
		condition.property,
		real_filter_make_string_array(condition.strings),
		condition.comparison_type.value,
		parser::position_tag_of(condition.comparison_type),
		condition.integer.map([](ast::common::integer_literal il) { return detail::evaluate(il); }),
		parser::position_tag_of(condition),
		set,
		diagnostics);
}

[[nodiscard]] boost::optional<influences_container>
real_filter_make_influences_container(
	const ast::rf::influence_spec& spec,
	lang::position_tag condition_origin,
	diagnostics_container& diagnostics)
{
	using result_type = boost::optional<influences_container>;

	return spec.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::rf::influence_literal_array& array) -> result_type {
			const auto num_inf = static_cast<int>(array.size());

			if (num_inf < 1 || num_inf > lang::limits::max_filter_influences) {
				push_error_invalid_amount_of_arguments(
					1, lang::limits::max_filter_influences, num_inf, condition_origin, diagnostics);
				return boost::none;
			}

			influences_container influences;
			for (auto inf : array) {
				influences.emplace_back(detail::evaluate(inf));
			}

			return influences;
		},
		[](ast::rf::none_literal /* literal */) -> result_type {
			return influences_container();
		}
	));
}

[[nodiscard]] bool
real_filter_add_has_influence_condition(
	settings st,
	const ast::rf::has_influence_condition& condition,
	std::optional<lang::influences_condition>& target,
	diagnostics_container& diagnostics)
{
	const auto condition_origin = parser::position_tag_of(condition);
	return real_filter_make_influences_container(condition.spec, condition_origin, diagnostics)
		.map([&](influences_container influences) {
			return add_has_influence_condition(
				st,
				std::move(influences),
				condition.exact_match.required,
				condition_origin,
				target,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
real_filter_add_socket_spec_condition(
	settings st,
	const ast::rf::socket_spec_condition& condition,
	lang::condition_set& set,
	diagnostics_container& diagnostics)
{
	lang::socket_spec_condition::container_type specs;

	for (ast::common::literal_expression lit_expr : condition.literals) {
		boost::optional<lang::socket_spec> ss = detail::evaluate(st, lit_expr, diagnostics)
			.flat_map([&](lang::single_object so) { return detail::get_as_socket_spec(so, diagnostics); });

		if (ss)
			specs.push_back(*ss);
		else if (st.error_handling.stop_on_error)
			return false;
	}

	return add_socket_spec_condition(
		condition.links_matter,
		lang::socket_spec_condition{
			condition.comparison_type.value,
			std::move(specs),
			parser::position_tag_of(condition)
		},
		set,
		diagnostics);
}

} // namespace

namespace fs::compiler::detail
{

bool
spirit_filter_add_condition(
	settings st,
	const ast::sf::condition& condition,
	const symbol_table& symbols,
	lang::spirit_condition_set& set,
	diagnostics_container& diagnostics)
{
	return condition.apply_visitor(x3::make_lambda_visitor<bool>(
		[&](const ast::sf::autogen_condition& cond) {
			auto category = lang::item_category::_from_string_nothrow(cond.name.c_str());
			if (!category) {
				push_error_invalid_expression(parser::position_tag_of(cond.name), "invalid autogeneration", diagnostics);
			}

			return add_non_range_condition(
				lang::autogen_condition{*category, parser::position_tag_of(cond)},
				set.autogen,
				diagnostics
			);
		},
		[&](const ast::sf::price_comparison_condition& cond) {
			return spirit_filter_add_price_comparison_condition(st, cond, symbols, set, diagnostics);
		},
		[&](const ast::sf::rarity_comparison_condition& cond) {
			return spirit_filter_add_rarity_comparison_condition(st, cond, symbols, set.conditions, diagnostics);
		},
		[&](const ast::sf::numeric_comparison_condition& cond) {
			return spirit_filter_add_numeric_comparison_condition(st, cond, symbols, set.conditions, diagnostics);
		},
		[&](const ast::sf::string_array_condition& cond) {
			return spirit_filter_add_string_array_condition(st, cond, symbols, set.conditions, diagnostics);
		},
		[&](const ast::sf::ranged_string_array_condition& cond) {
			return spirit_filter_add_ranged_string_array_condition(st, cond, symbols, set.conditions, diagnostics);
		},
		[&](const ast::sf::has_influence_condition& cond) {
			return spirit_filter_add_has_influence_condition(st, cond, symbols, set.conditions, diagnostics);
		},
		[&](const ast::sf::socket_spec_condition& cond) {
			return spirit_filter_add_socket_spec_condition(st, cond, symbols, set.conditions, diagnostics);
		},
		[&](const ast::sf::boolean_condition& cond) {
			return spirit_filter_add_boolean_condition(st, cond, symbols, set.conditions, diagnostics);
		}
	));
}

bool
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::condition_set& condition_set,
	diagnostics_container& diagnostics)
{
	return condition.apply_visitor(x3::make_lambda_visitor<bool>(
		[&](const ast::rf::rarity_condition& cond) {
			return add_range_condition(
				cond.comparison_type.value,
				evaluate(cond.rarity),
				parser::position_tag_of(cond),
				condition_set.rarity,
				diagnostics);
		},
		[&](const ast::rf::numeric_condition& cond) {
			return real_filter_add_numeric_comparison_condition(cond, condition_set, diagnostics);
		},
		[&](const ast::rf::string_array_condition& cond) {
			return real_filter_add_string_array_condition(cond, condition_set, diagnostics);
		},
		[&](const ast::rf::ranged_string_array_condition& cond) {
			return real_filter_add_ranged_string_array_condition(cond, condition_set, diagnostics);
		},
		[&](const ast::rf::has_influence_condition& cond) {
			return real_filter_add_has_influence_condition(st, cond, condition_set.has_influence, diagnostics);
		},
		[&](const ast::rf::socket_spec_condition& cond) {
			return real_filter_add_socket_spec_condition(st, cond, condition_set, diagnostics);
		},
		[&](const ast::rf::boolean_condition& cond) {
			return add_boolean_condition(
				evaluate(cond.value),
				parser::position_tag_of(cond),
				cond.property,
				condition_set,
				diagnostics);
		}
	));
}

}
