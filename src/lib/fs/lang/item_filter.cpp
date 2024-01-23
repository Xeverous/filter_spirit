#include <algorithm>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/type_traits.hpp>

#include <utility>
#include <ostream>
#include <type_traits>

namespace fs::lang
{

namespace
{

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

void item_filter_block::print(std::ostream& output_stream) const
{
	if (!is_valid())
		return;

	if (visibility.policy == item_visibility_policy::show)
		output_stream << keywords::rf::show;
	else
		output_stream << keywords::rf::hide;
	output_stream << '\n';

	conditions.print(output_stream);
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
