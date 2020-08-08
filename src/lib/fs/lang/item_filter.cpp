#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/assert.hpp>

#include <ostream>

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
	output_stream << '\n';
}

namespace {

// return whether a given property satisfies strings_condition
bool strings_condition_test(const strings_condition& condition, const std::string& property)
{
	for (const auto& str : condition.strings) {
		// TODO in-game filters ignore diacritics,
		// here 2 UTF-8 strings are compared strictly
		if (condition.exact_match_required) {
			if (property == str)
				return true;
		}
		else {
			if (utility::contains(property, str))
				return true;
		}
	}

	return false;
}

bool strings_condition_test(const strings_condition& condition, const std::vector<std::string>& properties)
{
	for (const auto& prop : properties) {
		if (strings_condition_test(condition, prop))
			return true;
	}

	return false;
}

bool influences_condition_test(influences_condition condition, influence_info item_influence)
{
	// HasInfluence condition is a bit different.
	// 1) HasInfluence None is an official feature and will match items with exactly no influence
	// 2) It behaves differently with ==:
	//    - If there is == (exact matching), it will only match items with all specified influences.
	//    - If there is nothing or =, it will match an item with at least one of specified influences.
	if (condition.influence.is_none()) {
		return item_influence.is_none();
	}

	if (condition.exact_match_required) {
		if (condition.influence.shaper && !item_influence.shaper)
			return false;

		if (condition.influence.elder && !item_influence.elder)
			return false;

		if (condition.influence.crusader && !item_influence.crusader)
			return false;

		if (condition.influence.redeemer && !item_influence.redeemer)
			return false;

		if (condition.influence.hunter && !item_influence.hunter)
			return false;

		if (condition.influence.warlord && !item_influence.warlord)
			return false;

		return true;
	}
	else {
		if (condition.influence.shaper && item_influence.shaper)
			return true;

		if (condition.influence.elder && item_influence.elder)
			return true;

		if (condition.influence.crusader && item_influence.crusader)
			return true;

		if (condition.influence.redeemer && item_influence.redeemer)
			return true;

		if (condition.influence.hunter && item_influence.hunter)
			return true;

		if (condition.influence.warlord && item_influence.warlord)
			return true;

		return false;
	}
}

action_set default_item_style(const item& /* itm */)
{
	action_set result;

	// initial color values taken from:
	// https://www.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/fjvo44v/
	result.set_text_color   = color_action{color{integer{200}, integer{200}, integer{200}, integer{255}}, {}};
	result.set_border_color = color_action{color{integer{200}, integer{200}, integer{200}, integer{255}}, {}};
	result.set_background_color = color_action{color{integer{0}, integer{0}, integer{0}, integer{240}}, {}};

	// TODO more styles depending on item rarity and class
	// ...

	return result;
}

}

item_filtering_result pass_item_through_filter(const item& itm, const item_filter& filter, int area_level)
{
	std::vector<condition_set_match_result> match_history;
	// each item is expected to traverse half of the filter on average
	match_history.reserve(filter.blocks.size() / 2);

	action_set item_style = default_item_style(itm);

	for (const item_filter_block& block : filter.blocks) {
		BOOST_ASSERT(block.conditions.is_valid());

		condition_set_match_result match_result;
		const condition_set& cs = block.conditions;

		const auto test_range_condition = [](auto range_condition, auto item_property_value, std::optional<bool>& match_result) {
			if (!range_condition.has_bound())
				return;

			match_result = range_condition.includes(item_property_value);
		};

		test_range_condition(cs.item_level,     itm.item_level,      match_result.item_level);
		test_range_condition(cs.drop_level,     itm.drop_level,      match_result.drop_level);
		test_range_condition(cs.quality,        itm.quality,         match_result.quality);
		test_range_condition(cs.rarity,         itm.rarity_,         match_result.rarity);
		test_range_condition(cs.links,          itm.sockets.links(), match_result.links);
		test_range_condition(cs.height,         itm.height,          match_result.height);
		test_range_condition(cs.width,          itm.width,           match_result.width);
		test_range_condition(cs.stack_size,     itm.stack_size,      match_result.stack_size);
		test_range_condition(cs.gem_level,      itm.gem_level,       match_result.gem_level);
		test_range_condition(cs.map_tier,       itm.map_tier,        match_result.map_tier);
		test_range_condition(cs.area_level,     area_level,          match_result.area_level);
		test_range_condition(cs.corrupted_mods, itm.corrupted_mods,  match_result.corrupted_mods);

		const auto test_strings_condition = [](
			const std::optional<strings_condition>& condition,
			const auto& item_property_value,
			std::optional<bool>& match_result)
		{
			if (!condition)
				return;

			match_result = strings_condition_test(*condition, item_property_value);
		};

		test_strings_condition(cs.class_, itm.class_, match_result.class_);
		test_strings_condition(cs.base_type, itm.base_type, match_result.base_type);
		test_strings_condition(cs.has_explicit_mod, itm.explicit_mods, match_result.has_explicit_mod);
		test_strings_condition(cs.has_enchantment, itm.enchantments_labirynth, match_result.has_enchantment);
		test_strings_condition(cs.enchantment_passive_node, itm.enchantments_passive_nodes, match_result.enchantment_passive_node);

		// Prophecy condition is a bit different. It does not use "Prophecy" class but instead treats
		// the item base type name as the prophecy name. If the item is an itemised prophecy, then
		// the base type is compared, otherwise item can not match Prophecy condition.
		if (cs.prophecy) {
			match_result.prophecy = itm.is_prophecy && strings_condition_test(*cs.prophecy, itm.base_type);
		}

		if (cs.has_influence) {
			match_result.has_influence = influences_condition_test(*cs.has_influence, itm.influence);
		}

		const auto test_boolean_condition = [](
			std::optional<boolean_condition> condition,
			bool item_property_value,
			std::optional<bool>& match_result)
		{
			if (!condition)
				return;

			match_result = (*condition).value.value == item_property_value;
		};

		test_boolean_condition(cs.is_identified, itm.is_identified, match_result.is_identified);
		test_boolean_condition(cs.is_corrupted, itm.is_corrupted, match_result.is_corrupted);
		test_boolean_condition(cs.is_mirrored, itm.is_mirrored, match_result.is_mirrored);
		test_boolean_condition(cs.is_elder_item, itm.influence.elder, match_result.is_elder_item);
		test_boolean_condition(cs.is_shaper_item, itm.influence.shaper, match_result.is_shaper_item);
		test_boolean_condition(cs.is_fractured_item, itm.is_fractured_item, match_result.is_fractured_item);
		test_boolean_condition(cs.is_synthesised_item, itm.is_synthesised_item, match_result.is_synthesised_item);
		test_boolean_condition(cs.is_enchanted, !itm.enchantments_labirynth.empty() || !itm.enchantments_passive_nodes.empty() || !itm.annointments.empty(), match_result.is_enchanted);
		test_boolean_condition(cs.is_shaped_map, itm.is_shaped_map, match_result.is_shaped_map);
		test_boolean_condition(cs.is_elder_map, itm.is_elder_map, match_result.is_elder_map);
		test_boolean_condition(cs.is_blighted_map, itm.is_blighted_map, match_result.is_blighted_map);

		// TODO sockets matching

		match_history.push_back(match_result);

		if (match_result.is_successful()) {
			item_style.override_with(block.actions);

			return item_filtering_result{
				block.visibility,
				item_style,
				std::move(match_history)
			};
		}
	}

	return item_filtering_result{
		item_visibility{true, {}},
		item_style,
		std::move(match_history)
	};
}

}
