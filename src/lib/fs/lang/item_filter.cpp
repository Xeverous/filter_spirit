#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/string_helpers.hpp>
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
	for (const auto& str : condition.strings) {
		/*
		 * TODO
		 * 1. In-game filters ignore diacritics, here 2 UTF-8 strings are compared strictly.
		 * 2. strings_condition stores std::string, not lang::string which means there is
		 *    no access to the string's origin here. Right now just duplicate condition's
		 *    origin to make it compile.
		 */
		if (condition.exact_match_required) {
			if (property == str)
				return condition_match_result::success(condition.origin, condition.origin);
		}
		else {
			if (utility::contains(property, str))
				return condition_match_result::success(condition.origin, condition.origin);
		}
	}

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
	const std::optional<strings_condition>& opt_condition,
	const std::vector<std::string>& properties)
{
	if (!opt_condition)
		return std::nullopt;

	const strings_condition& condition = *opt_condition;

	for (const auto& prop : properties) {
		condition_match_result result = test_strings_condition_impl(condition, prop);
		if (result.is_successful())
			return result;
	}

	return condition_match_result::failure(condition.origin);
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

item_style default_item_style(const item& itm)
{
	item_style result;

	if (itm.rarity_ == rarity_type::magic)
		result.text_color = color_action{color{integer{136}, integer{136}, integer{255}, integer{255}}, {}};
	else if (itm.rarity_ == rarity_type::rare)
		result.text_color = color_action{color{integer{255}, integer{255}, integer{119}, integer{255}}, {}};
	else if (itm.rarity_ == rarity_type::unique)
		result.text_color = color_action{color{integer{175}, integer{ 95}, integer{ 28}, integer{255}}, {}};

	if (itm.class_ == item_class_names::currency_stackable || itm.class_ == item_class_names::currency_delve)
		result.text_color = color_action{color{integer{170}, integer{158}, integer{129}, integer{255}}, {}};
	else if (itm.class_ == item_class_names::divination_card)
		result.text_color = color_action{color{integer{  0}, integer{186}, integer{255}, integer{255}}, {}};
	else if (itm.class_ == item_class_names::gem_active || itm.class_ == item_class_names::gem_support)
		result.text_color = color_action{color{integer{ 13}, integer{162}, integer{155}, integer{255}}, {}};
	else if (itm.is_prophecy)
		result.text_color = color_action{color{integer{181}, integer{ 75}, integer{255}, integer{255}}, {}};
	else if (itm.class_ == item_class_names::quest_item)
		result.text_color = color_action{color{integer{ 71}, integer{230}, integer{ 53}, integer{255}}, {}};
	// maps are both in these class names and in rarities - border color is the same as text color
	else if (itm.class_ == item_class_names::map || itm.class_ == item_class_names::map_fragment)
		result.border_color = result.text_color;

	return result;
}

} // namespace

namespace fs::lang
{

void item_filter_block::generate(std::ostream& output_stream) const
{
	if (!conditions.is_valid())
		return;

	if (visibility.show)
		output_stream << keywords::rf::show;
	else
		output_stream << keywords::rf::hide;
	output_stream << '\n';

	conditions.generate(output_stream);
	actions.generate(output_stream);

	if (continuation.continue_)
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

	if (actions.disable_drop_sound)
		disable_drop_sound = *actions.disable_drop_sound;

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

		cs_result.class_                   = test_strings_condition(cs.class_,                   itm.class_);
		cs_result.base_type                = test_strings_condition(cs.base_type,                itm.base_type);
		cs_result.has_explicit_mod         = test_strings_condition(cs.has_explicit_mod,         itm.explicit_mods);
		cs_result.has_enchantment          = test_strings_condition(cs.has_enchantment,          itm.enchantments_labirynth);
		cs_result.enchantment_passive_node = test_strings_condition(cs.enchantment_passive_node, itm.enchantments_passive_nodes);

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
			!itm.enchantments_labirynth.empty() || !itm.enchantments_passive_nodes.empty() || !itm.annointments.empty());
		cs_result.is_shaped_map        = test_boolean_condition(cs.is_shaped_map,        itm.is_shaped_map);
		cs_result.is_elder_map         = test_boolean_condition(cs.is_elder_map,         itm.is_elder_map);
		cs_result.is_blighted_map      = test_boolean_condition(cs.is_blighted_map,      itm.is_blighted_map);
		cs_result.is_replica           = test_boolean_condition(cs.is_replica,           itm.is_replica);
		cs_result.is_alternate_quality = test_boolean_condition(cs.is_alternate_quality, itm.is_alternate_quality);

		// TODO gem quality type matching
		// TODO sockets matching

		block_match_result block_result;
		block_result.condition_set_result = std::move(cs_result);

		bool stop_filtering = false;
		if (block_result.condition_set_result.is_successful()) {
			style.override_with(block.actions);
			style.visibility = block.visibility;

			if (block.continuation.continue_)
				block_result.continue_origin = block.continuation.origin;
			else
				stop_filtering = true;
		}

		match_history.push_back(std::move(block_result));

		if (stop_filtering)
			break;
	}

	return item_filtering_result{style, std::move(match_history)};
}

}
