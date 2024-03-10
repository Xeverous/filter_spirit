#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/type_traits.hpp>

#include <algorithm>
#include <utility>
#include <ostream>
#include <type_traits>

namespace fs::lang
{

namespace
{

lang::integer make_int(int val)
{
	return integer{val, no_origin()};
};

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
		result.text_color = color_action{color{make_int(136), make_int(136), make_int(255), make_int(255)}, no_origin()};
	else if (itm.rarity_ == rarity_type::rare)
		result.text_color = color_action{color{make_int(255), make_int(255), make_int(119), make_int(255)}, no_origin()};
	else if (itm.rarity_ == rarity_type::unique)
		result.text_color = color_action{color{make_int(175), make_int( 96), make_int( 37), make_int(255)}, no_origin()};

	if (itm.class_ == item_class_names::currency_stackable || itm.class_ == item_class_names::resonators)
		result.text_color = color_action{color{make_int(170), make_int(158), make_int(130), make_int(255)}, no_origin()};
	else if (itm.class_ == item_class_names::divination_card)
		result.text_color = color_action{color{make_int( 14), make_int(186), make_int(255), make_int(255)}, no_origin()};
	else if (itm.class_ == item_class_names::gems_active || itm.class_ == item_class_names::gems_support)
		result.text_color = color_action{color{make_int( 27), make_int(162), make_int(155), make_int(255)}, no_origin()};
	else if (itm.class_ == item_class_names::quest_items)
		result.text_color = color_action{color{make_int( 74), make_int(230), make_int( 58), make_int(255)}, no_origin()};
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

void item_filter_block::print(std::ostream& output_stream) const
{
	if (!is_valid())
		return;

	switch (visibility.policy) {
		case item_visibility_policy::show:
			output_stream << keywords::rf::show;
			break;
		case item_visibility_policy::hide:
			output_stream << keywords::rf::hide;
			break;
		case item_visibility_policy::minimal:
			output_stream << keywords::rf::minimal;
			break;
		case item_visibility_policy::discard:
			return;
	}

	output_stream << '\n';

	conditions.print(output_stream);
	actions.print(output_stream);

	if (continuation.origin)
		output_stream << '\t' << keywords::rf::continue_ << '\n';

	output_stream << '\n';
}

item_style::item_style()
: visibility{true, no_origin()}
// initial color values taken from:
// https://www.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/fjvo44v/
, text_color      {color{make_int(200), make_int(200), make_int(200), make_int(255)}, no_origin()}
, background_color{color{make_int(  0), make_int(  0), make_int(  0), make_int(240)}, no_origin()}
, font_size{make_int(32), no_origin()}
{
}

void item_style::override_with(const action_set& actions)
{
	if (actions.text_color)
		text_color = *actions.text_color;

	if (actions.border_color)
		border_color = *actions.border_color;

	if (actions.background_color)
		background_color = *actions.background_color;

	if (actions.font_size)
		font_size = *actions.font_size;

	if (actions.effect)
		effect = *actions.effect;

	if (actions.minimap_icon)
		minimap_icon = *actions.minimap_icon;

	if (actions.alert_sound)
		alert_sound = *actions.alert_sound;

	if (actions.switch_drop_sound)
		switch_drop_sound = *actions.switch_drop_sound;

	if (actions.switch_drop_sound_if_alert_sound)
		switch_drop_sound_if_alert_sound = *actions.switch_drop_sound_if_alert_sound;
}

block_match_result item_filter_block::test_item(const item& itm, int area_level) const
{
	block_match_result result;
	result.continue_origin = continuation.origin;

	if (!is_valid()) {
		result.status = block_match_status::failure_invalid_block;
		return result;
	}

	for (const auto& cond : conditions.conditions)
		result.match_results.push_back(cond->test_item(itm, area_level));

	const bool is_successful = std::all_of(
		result.match_results.begin(),
		result.match_results.end(),
		[](const condition_match_result& result) { return result.is_successful(); });
	result.status = is_successful ? block_match_status::success : block_match_status::failure_mismatch;

	return result;
}

item_filtering_result pass_item_through_filter(const item& itm, const item_filter& filter, int area_level)
{
	std::vector<block_match_result> match_history;
	// each item is expected to traverse half of the filter on average
	match_history.reserve(filter.blocks.size() / 2);

	item_style style = default_item_style(itm);

	for (const item_filter_block& block : filter.blocks) {
		block_match_result block_result = block.test_item(itm, area_level);

		bool stop_filtering = false;
		if (block_result.is_successful()) {
			style.override_with(block.actions);
			style.visibility = to_item_visibility_style(block.visibility);

			if (block.continuation.origin) {
				FS_ASSERT(is_valid(*block.continuation.origin));
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

} // namespace fs::lang
