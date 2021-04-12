#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/type_traits.hpp>

#include <utility>
#include <ostream>
#include <type_traits>

namespace {

using namespace fs;
using namespace fs::lang;

template <typename T, typename U> [[nodiscard]]
range_condition_match_result test_range_condition(range_condition<T> condition, U item_property_value)
{
	static_assert(std::is_same_v<decltype(T::value), traits::remove_cvref_t<U>>);

	range_condition_match_result result;

	if (condition.lower_bound) {
		result.lower_bound = condition_match_result(
			condition.test_lower_bound(item_property_value),
			(*condition.lower_bound).origin,
			(*condition.lower_bound).value.origin);
	}

	if (condition.upper_bound) {
		result.upper_bound = condition_match_result(
			condition.test_upper_bound(item_property_value),
			(*condition.upper_bound).origin,
			(*condition.upper_bound).value.origin);
	}

	return result;
}

[[nodiscard]] condition_match_result
test_strings_condition_impl(
	const strings_condition& condition,
	const std::string& property)
{
	if (const lang::string* str = condition.find_match(property); str)
		return condition_match_result::success(condition.origin, str->origin);
	else
		return condition_match_result::failure(condition.origin);
}

[[nodiscard]] std::optional<condition_match_result>
test_strings_condition(
	const std::optional<strings_condition>& opt_condition,
	const std::string& property)
{
	if (!opt_condition)
		return std::nullopt;

	return test_strings_condition_impl(*opt_condition, property);
}

[[nodiscard]] std::optional<condition_match_result>
test_strings_condition(
	const strings_condition& condition,
	const std::vector<std::string>& properties)
{
	for (const auto& prop : properties) {
		condition_match_result result = test_strings_condition_impl(condition, prop);
		if (result.is_successful())
			return result;
	}

	return condition_match_result::failure(condition.origin);
}

[[nodiscard]] std::optional<condition_match_result>
test_strings_condition(
	const std::optional<strings_condition>& opt_condition,
	const std::vector<std::string>& properties)
{
	if (!opt_condition)
		return std::nullopt;

	return test_strings_condition(*opt_condition, properties);
}

[[nodiscard]] std::optional<condition_match_result>
test_ranged_strings_condition(
	const std::optional<ranged_strings_condition>& opt_condition,
	const std::vector<std::string>& properties)
{
	if (!opt_condition)
		return std::nullopt;

	const ranged_strings_condition& condition = *opt_condition;
	const integer_range_condition& integer_cond = condition.integer_cond;
	const strings_condition& strings_cond = condition.strings_cond;

	if (integer_cond.has_bound()) {
		int matches = 0;

		for (const auto& prop : properties) {
			if (strings_cond.find_match(prop) != nullptr)
				++matches;
		}

		range_condition_match_result result = test_range_condition(integer_cond, matches);
		if (integer_cond.is_exact()) {
			FS_ASSERT(result.lower_bound.has_value());
			FS_ASSERT(result.upper_bound.has_value());

			if ((*result.lower_bound).is_successful() && (*result.upper_bound).is_successful())
				return condition_match_result::success(condition.origin);
			else
				return condition_match_result::failure(condition.origin);
		}
		else if (integer_cond.lower_bound) {
			return result.lower_bound;
		}
		else if (integer_cond.upper_bound) {
			return result.upper_bound;
		}
		else {
			FS_ASSERT_MSG(false, "unreachable: if integer condition has a bound, it should not get here");
			return condition_match_result::failure(condition.origin); // silence no-return warning
		}
	}
	else {
		return test_strings_condition(strings_cond, properties);
	}
}

[[nodiscard]] std::optional<condition_match_result>
test_prophecy_condition(
	const std::optional<strings_condition>& opt_condition,
	const item& itm)
{
	if (!opt_condition)
		return std::nullopt;
	/*
	 * Prophecy condition is a bit different. It is not a boolean condition nor does it check
	 * for "Prophecy" class. Instead it treats the item base type name as the prophecy name.
	 * If the item is an itemised prophecy, then the base type is compared (as if in BaseType
	 * condition), otherwise condition fails.
	 */
	const strings_condition& condition = *opt_condition;

	if (!itm.is_prophecy)
		return condition_match_result::failure(condition.origin);

	return test_strings_condition_impl(condition, itm.base_type);
}

[[nodiscard]] std::optional<condition_match_result>
test_influences_condition(
	const std::optional<influences_condition>& opt_condition,
	influence_info item_influence)
{
	if (!opt_condition)
		return std::nullopt;
	/*
	 * HasInfluence condition is a bit different.
	 * 1) HasInfluence None is an official feature and will match items with exactly no influence
	 * 2) It behaves differently with ==:
	 *    - If there is == (exact matching), it will only match items with all specified influences.
	 *    - If there is nothing or =, it will match an item with at least one of specified influences.
	 */
	const influences_condition& condition = *opt_condition;

	if (condition.influence.is_none()) {
		if (item_influence.is_none()) // TODO for positive case, return origin of "None"
			return condition_match_result::success(condition.origin, condition.origin);
		else
			return condition_match_result::failure(condition.origin);
	}

	// TODO return better origins on successful paths
	if (condition.exact_match_required) {
		if (condition.influence.shaper   && !item_influence.shaper)
			return condition_match_result::failure(condition.origin);

		if (condition.influence.elder    && !item_influence.elder)
			return condition_match_result::failure(condition.origin);

		if (condition.influence.crusader && !item_influence.crusader)
			return condition_match_result::failure(condition.origin);

		if (condition.influence.redeemer && !item_influence.redeemer)
			return condition_match_result::failure(condition.origin);

		if (condition.influence.hunter   && !item_influence.hunter)
			return condition_match_result::failure(condition.origin);

		if (condition.influence.warlord  && !item_influence.warlord)
			return condition_match_result::failure(condition.origin);

		return condition_match_result::success(condition.origin, condition.origin);
	}
	else {
		if (condition.influence.shaper   && item_influence.shaper)
			return condition_match_result::success(condition.origin, condition.origin);

		if (condition.influence.elder    && item_influence.elder)
			return condition_match_result::success(condition.origin, condition.origin);

		if (condition.influence.crusader && item_influence.crusader)
			return condition_match_result::success(condition.origin, condition.origin);

		if (condition.influence.redeemer && item_influence.redeemer)
			return condition_match_result::success(condition.origin, condition.origin);

		if (condition.influence.hunter   && item_influence.hunter)
			return condition_match_result::success(condition.origin, condition.origin);

		if (condition.influence.warlord  && item_influence.warlord)
			return condition_match_result::success(condition.origin, condition.origin);

		return condition_match_result::failure(condition.origin);
	}
}

[[nodiscard]] std::optional<condition_match_result>
test_gem_quality_type_condition(
	const std::optional<gem_quality_type_condition>& opt_condition,
	lang::gem_quality_type_type item_gqt)
{
	if (!opt_condition)
		return std::nullopt;

	const gem_quality_type_condition& condition = *opt_condition;

	for (lang::gem_quality_type gqt : condition.values) {
		if (item_gqt == gqt.value)
			return condition_match_result::success(condition.origin, gqt.origin);
	}

	return condition_match_result::failure(condition.origin);
}

[[nodiscard]] std::optional<condition_match_result>
test_boolean_condition(
	std::optional<boolean_condition> opt_condition,
	bool item_property_value)
{
	if (!opt_condition)
		return std::nullopt;

	const boolean_condition& condition = *opt_condition;

	if (condition.value.value == item_property_value)
		return condition_match_result::success(condition.origin, condition.value.origin);
	else
		return condition_match_result::failure(condition.origin);
}

/*
 * Sockets and SocketGroup are one of the most unintuitive and complex conditions.
 * There are multiple inconsistencies:
 * - items with no sockets get special treatment, as if they had one link-group of size 0
 *   (which is different from having no link-groups)
 * - an item can match < but not <=
 * - an item can match > but not >=
 * - < and > use OR operation for subrequirements, other comparisons use AND (function C)
 *   (thus "< 5G" is equivalent to "< 5 G" but "<= 5G" is not equivalent to "<= 5 G")
 * - soft-equal (no comparison symbol or only 1 "=" character):
 *   - uses == to compare sockets/link count (function A)
 *   - uses <= for color counts (function B)
 *
 * Sockets     <  5GGG # less than 5        sockets OR  less than 3x G socket
 * Sockets     <= 5GGG # at most   5        sockets AND at most   3x G socket
 * Sockets        5GGG # exactly   5        sockets AND at least  3x G socket
 * Sockets     == 5GGG # exactly   5        sockets AND exactly   3x G socket
 * Sockets     >= 5GGG # at least  5        sockets AND at least  3x G socket
 * Sockets     >  5GGG # more than 5        sockets OR  more than 3x G socket
 * SocketGroup <  5GGG # less than 5 linked sockets OR  less than 3x G linked within any  group
 * SocketGroup <= 5GGG # at most   5 linked sockets AND at most   3x G linked within such group
 * SocketGroup    5GGG # exactly   5 linked sockets AND at least  3x G linked within such group
 * SocketGroup == 5GGG # exactly   5 linked sockets AND exactly   3x G linked within such group
 * SocketGroup >= 5GGG # at least  5 linked sockets AND at least  3x G linked within such group
 * SocketGroup >  5GGG # more than 5 linked sockets OR  more than 3x G linked within any  group
 */
[[nodiscard]] bool // (A)
test_socket_or_link_count(std::optional<int> opt_req, comparison_type comparison, int item_stat)
{
	if (!opt_req)
		return true;

	const int req = *opt_req;

	if (comparison == comparison_type::less)
		return item_stat < req;
	else if (comparison == comparison_type::less_equal)
		return item_stat <= req;
	else if (comparison == comparison_type::greater)
		return item_stat > req;
	else if (comparison == comparison_type::greater_equal)
		return item_stat >= req;
	else
		return item_stat == req;
}

[[nodiscard]] std::optional<bool> // (B)
test_color_count(int req, comparison_type comparison, int item_stat)
{
	/*
	 * It is impossible to query for count == 0 of specific socket color.
	 * This is because for each required specific color, a specific letter
	 * needs to be written. Multiple letters increase required count.
	 * No letters means no requirement therefore the caller must check
	 * for results elsewhere. There is no good default here, return nothing.
	 */
	if (req == 0)
		return std::nullopt;

	if (comparison == comparison_type::less)
		return item_stat < req;
	else if (comparison == comparison_type::less_equal)
		return item_stat <= req;
	else if (comparison == comparison_type::greater)
		return item_stat > req;
	else if (comparison == comparison_type::greater_equal || comparison == comparison_type::equal_soft)
		return item_stat >= req;
	else
		return item_stat == req;
}

[[nodiscard]] bool
test_color_count_all_groups(const socket_info& item_sockets, socket_color c, int req, comparison_type comparison)
{
	for (const linked_sockets& group : item_sockets.groups) {
		if (test_color_count(req, comparison, group.count_of(c)).value_or(false))
			return true;
	}

	return false;
}

[[nodiscard]] bool // (C)
is_sockets_condition_using_or(comparison_type comparison)
{
	return comparison == comparison_type::less || comparison == comparison_type::greater;
}

[[nodiscard]] bool
test_sockets_condition(
	comparison_type comparison,
	socket_spec ss,
	const socket_info& item_sockets)
{
	if (item_sockets.groups.empty()) {
		return test_socket_or_link_count(ss.num, comparison, 0)
			&& test_color_count(ss.r, comparison, 0).value_or(true)
			&& test_color_count(ss.g, comparison, 0).value_or(true)
			&& test_color_count(ss.b, comparison, 0).value_or(true)
			&& test_color_count(ss.w, comparison, 0).value_or(true)
			&& test_color_count(ss.a, comparison, 0).value_or(true)
			&& test_color_count(ss.d, comparison, 0).value_or(true);
	}

	if (is_sockets_condition_using_or(comparison)) {
		return test_socket_or_link_count(ss.num, comparison, item_sockets.sockets())
			|| test_color_count(ss.r, comparison, item_sockets.count_of(socket_color::r)).value_or(false)
			|| test_color_count(ss.g, comparison, item_sockets.count_of(socket_color::g)).value_or(false)
			|| test_color_count(ss.b, comparison, item_sockets.count_of(socket_color::b)).value_or(false)
			|| test_color_count(ss.w, comparison, item_sockets.count_of(socket_color::w)).value_or(false)
			|| test_color_count(ss.a, comparison, item_sockets.count_of(socket_color::a)).value_or(false)
			|| test_color_count(ss.d, comparison, item_sockets.count_of(socket_color::d)).value_or(false);
	}
	else {
		return test_socket_or_link_count(ss.num, comparison, item_sockets.sockets())
			&& test_color_count(ss.r, comparison, item_sockets.count_of(socket_color::r)).value_or(true)
			&& test_color_count(ss.g, comparison, item_sockets.count_of(socket_color::g)).value_or(true)
			&& test_color_count(ss.b, comparison, item_sockets.count_of(socket_color::b)).value_or(true)
			&& test_color_count(ss.w, comparison, item_sockets.count_of(socket_color::w)).value_or(true)
			&& test_color_count(ss.a, comparison, item_sockets.count_of(socket_color::a)).value_or(true)
			&& test_color_count(ss.d, comparison, item_sockets.count_of(socket_color::d)).value_or(true);
	}
}

[[nodiscard]] bool
test_socket_group_condition(
	comparison_type comparison,
	socket_spec ss,
	const socket_info& item_sockets)
{
	if (item_sockets.groups.empty()) {
		return test_socket_or_link_count(ss.num, comparison, 0)
			&& test_color_count(ss.r, comparison, 0).value_or(true)
			&& test_color_count(ss.g, comparison, 0).value_or(true)
			&& test_color_count(ss.b, comparison, 0).value_or(true)
			&& test_color_count(ss.w, comparison, 0).value_or(true)
			&& test_color_count(ss.a, comparison, 0).value_or(true)
			&& test_color_count(ss.d, comparison, 0).value_or(true);
	}

	if (is_sockets_condition_using_or(comparison)) {
		return test_socket_or_link_count(ss.num, comparison, item_sockets.links())
			|| test_color_count_all_groups(item_sockets, socket_color::r, ss.r, comparison)
			|| test_color_count_all_groups(item_sockets, socket_color::g, ss.g, comparison)
			|| test_color_count_all_groups(item_sockets, socket_color::b, ss.b, comparison)
			|| test_color_count_all_groups(item_sockets, socket_color::w, ss.w, comparison)
			|| test_color_count_all_groups(item_sockets, socket_color::a, ss.a, comparison)
			|| test_color_count_all_groups(item_sockets, socket_color::d, ss.d, comparison);
	}
	else {
		for (const linked_sockets& group : item_sockets.groups) {
			const bool result = test_socket_or_link_count(ss.num, comparison, group.sockets.size())
				&& test_color_count(ss.r, comparison, group.count_of(socket_color::r)).value_or(true)
				&& test_color_count(ss.g, comparison, group.count_of(socket_color::g)).value_or(true)
				&& test_color_count(ss.b, comparison, group.count_of(socket_color::b)).value_or(true)
				&& test_color_count(ss.w, comparison, group.count_of(socket_color::w)).value_or(true)
				&& test_color_count(ss.a, comparison, group.count_of(socket_color::a)).value_or(true)
				&& test_color_count(ss.d, comparison, group.count_of(socket_color::d)).value_or(true);

			if (result)
				return true;
		}

		return false;
	}
}

[[nodiscard]] std::optional<condition_match_result>
test_socket_spec_condition(
	const std::optional<socket_spec_condition>& opt_condition,
	bool group_matters,
	const socket_info& item_sockets)
{
	if (!opt_condition)
		return std::nullopt;

	const socket_spec_condition& condition = *opt_condition;

	for (socket_spec ss : condition.values) {
		const auto result = [&]() {
			if (group_matters)
				return test_socket_group_condition(condition.comparison, ss, item_sockets);
			else
				return test_sockets_condition(condition.comparison, ss, item_sockets);
		}();

		if (result)
			return condition_match_result::success(condition.origin, ss.origin);
	}

	return condition_match_result::failure(condition.origin);
}

item_style default_item_style(const item& itm)
{
	/*
	 * There is no official info for these - all data here comes from own experiments and PoE wiki.
	 * I checked resulting colors without filter in the blackest place I could find - Delve without lights.
	 * Most of colors matched the ones from wiki (listed below) but surprisingly even on RGB 2 2 2
	 * (blackest background I managed to find in Delve) some differed by ~10 (gamma correction?). Because
	 * I often had value 254 I suspect my values were 1-off (or more), thus all values below are from wiki.
	 */
	item_style result;

	if (itm.rarity_ == rarity_type::magic)
		result.text_color = color_action{color{integer{136}, integer{136}, integer{255}, integer{255}}, {}};
	else if (itm.rarity_ == rarity_type::rare)
		result.text_color = color_action{color{integer{255}, integer{255}, integer{119}, integer{255}}, {}};
	else if (itm.rarity_ == rarity_type::unique)
		result.text_color = color_action{color{integer{175}, integer{ 96}, integer{ 37}, integer{255}}, {}};

	if (itm.class_ == item_class_names::currency_stackable || itm.class_ == item_class_names::resonators)
		result.text_color = color_action{color{integer{170}, integer{158}, integer{130}, integer{255}}, {}};
	else if (itm.class_ == item_class_names::divination_card)
		result.text_color = color_action{color{integer{ 14}, integer{186}, integer{255}, integer{255}}, {}};
	else if (itm.class_ == item_class_names::gems_active || itm.class_ == item_class_names::gems_support)
		result.text_color = color_action{color{integer{ 27}, integer{162}, integer{155}, integer{255}}, {}};
	else if (itm.is_prophecy)
		result.text_color = color_action{color{integer{181}, integer{ 75}, integer{255}, integer{255}}, {}};
	else if (itm.class_ == item_class_names::quest_items)
		result.text_color = color_action{color{integer{ 74}, integer{230}, integer{ 58}, integer{255}}, {}};
	// maps are both in these class names and in rarities - border color is the same as text color
	else if (itm.class_ == item_class_names::maps || itm.class_ == item_class_names::map_fragments)
		result.border_color = result.text_color;

	return result;
}

item_visibility_style to_item_visibility_style(item_visibility visibility)
{
	FS_ASSERT(visibility.policy != item_visibility_policy::discard);
	return {visibility.policy == item_visibility_policy::show, visibility.origin};
}

} // namespace

namespace fs::lang
{

void item_filter_block::generate(std::ostream& output_stream) const
{
	if (!conditions.is_valid() || visibility.policy == item_visibility_policy::discard)
		return;

	if (visibility.policy == item_visibility_policy::show)
		output_stream << keywords::rf::show;
	else
		output_stream << keywords::rf::hide;
	output_stream << '\n';

	conditions.generate(output_stream);
	actions.generate(output_stream);

	if (continuation.origin)
		output_stream << '\t' << keywords::rf::continue_ << '\n';

	output_stream << '\n';
}


item_style::item_style()
: visibility{true, {}}
// initial color values taken from:
// https://www.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/fjvo44v/
, text_color  {color{integer{200}, integer{200}, integer{200}, integer{255}}, {}}
, background_color{color{integer{0}, integer{0}, integer{0}, integer{240}}, {}}
, font_size{integer{32}, {}}
{
}

void item_style::override_with(const action_set& actions)
{
	if (actions.set_border_color)
		border_color = *actions.set_border_color;

	if (actions.set_text_color)
		text_color = *actions.set_text_color;

	if (actions.set_background_color)
		background_color = *actions.set_background_color;

	if (actions.set_font_size)
		font_size = *actions.set_font_size;

	if (actions.play_alert_sound)
		play_alert_sound = *actions.play_alert_sound;

	if (actions.switch_drop_sound)
		switch_drop_sound = *actions.switch_drop_sound;

	if (actions.minimap_icon)
		minimap_icon = *actions.minimap_icon;

	if (actions.play_effect)
		play_effect = *actions.play_effect;
}

item_filtering_result pass_item_through_filter(const item& itm, const item_filter& filter, int area_level)
{
	std::vector<block_match_result> match_history;
	// each item is expected to traverse half of the filter on average
	match_history.reserve(filter.blocks.size() / 2);

	item_style style = default_item_style(itm);

	for (const item_filter_block& block : filter.blocks) {
		// TODO these should not be here - the function should not expect item_filter with extra invariants
		// instead of firing asserts, it should produce more detailed item_filtering_result
		FS_ASSERT(block.visibility.policy != item_visibility_policy::discard);
		FS_ASSERT(block.conditions.is_valid());

		const condition_set& cs = block.conditions;
		condition_set_match_result cs_result;

		cs_result.item_level     = test_range_condition(cs.item_level,     itm.item_level);
		cs_result.drop_level     = test_range_condition(cs.drop_level,     itm.drop_level);
		cs_result.quality        = test_range_condition(cs.quality,        itm.quality);
		cs_result.rarity         = test_range_condition(cs.rarity,         itm.rarity_);
		cs_result.linked_sockets = test_range_condition(cs.linked_sockets, itm.sockets.links());
		cs_result.height         = test_range_condition(cs.height,         itm.height);
		cs_result.width          = test_range_condition(cs.width,          itm.width);
		cs_result.stack_size     = test_range_condition(cs.stack_size,     itm.stack_size);
		cs_result.gem_level      = test_range_condition(cs.gem_level,      itm.gem_level);
		cs_result.map_tier       = test_range_condition(cs.map_tier,       itm.map_tier);
		cs_result.area_level     = test_range_condition(cs.area_level,     area_level);
		cs_result.corrupted_mods = test_range_condition(cs.corrupted_mods, itm.corrupted_mods);
		cs_result.enchantment_passive_num = test_range_condition(cs.enchantment_passive_num, 0); // TODO implement item support for it

		cs_result.class_                   = test_strings_condition(cs.class_,                   itm.class_);
		cs_result.base_type                = test_strings_condition(cs.base_type,                itm.base_type);
		cs_result.enchantment_passive_node = test_strings_condition(cs.enchantment_passive_node, itm.enchantments_passive_nodes);

		cs_result.has_explicit_mod         = test_ranged_strings_condition(cs.has_explicit_mod,  itm.explicit_mods);
		cs_result.has_enchantment          = test_ranged_strings_condition(cs.has_enchantment,   itm.enchantments_labyrinth);

		cs_result.prophecy = test_prophecy_condition(cs.prophecy, itm);

		cs_result.has_influence = test_influences_condition(cs.has_influence, itm.influence);

		cs_result.is_identified        = test_boolean_condition(cs.is_identified,        itm.is_identified);
		cs_result.is_corrupted         = test_boolean_condition(cs.is_corrupted,         itm.is_corrupted);
		cs_result.is_mirrored          = test_boolean_condition(cs.is_mirrored,          itm.is_mirrored);
		cs_result.is_elder_item        = test_boolean_condition(cs.is_elder_item,        itm.influence.elder);
		cs_result.is_shaper_item       = test_boolean_condition(cs.is_shaper_item,       itm.influence.shaper);
		cs_result.is_fractured_item    = test_boolean_condition(cs.is_fractured_item,    itm.is_fractured_item);
		cs_result.is_synthesised_item  = test_boolean_condition(cs.is_synthesised_item,  itm.is_synthesised_item);
		cs_result.is_enchanted         = test_boolean_condition(cs.is_enchanted, // yes, this one reacts to any of these
			!itm.enchantments_labyrinth.empty() || !itm.enchantments_passive_nodes.empty() || !itm.annointments.empty());
		cs_result.is_shaped_map        = test_boolean_condition(cs.is_shaped_map,        itm.is_shaped_map);
		cs_result.is_elder_map         = test_boolean_condition(cs.is_elder_map,         itm.is_elder_map);
		cs_result.is_blighted_map      = test_boolean_condition(cs.is_blighted_map,      itm.is_blighted_map);
		cs_result.is_replica           = test_boolean_condition(cs.is_replica,           itm.is_replica);
		cs_result.is_alternate_quality = test_boolean_condition(cs.is_alternate_quality, itm.is_alternate_quality);

		cs_result.gem_quality_type = test_gem_quality_type_condition(cs.gem_quality_type, itm.gem_quality_type);

		cs_result.sockets      = test_socket_spec_condition(cs.sockets,      false, itm.sockets);
		cs_result.socket_group = test_socket_spec_condition(cs.socket_group, true,  itm.sockets);

		block_match_result block_result;
		block_result.condition_set_result = std::move(cs_result);

		bool stop_filtering = false;
		if (block_result.condition_set_result.is_successful()) {
			style.override_with(block.actions);
			style.visibility = to_item_visibility_style(block.visibility);

			if (block.continuation.origin) {
				FS_ASSERT(is_valid(*block.continuation.origin));
				block_result.continue_origin = *block.continuation.origin;
			}
			else {
				stop_filtering = true;
			}
		}

		match_history.push_back(std::move(block_result));

		if (stop_filtering)
			break;
	}

	return item_filtering_result{style, std::move(match_history)};
}

}
