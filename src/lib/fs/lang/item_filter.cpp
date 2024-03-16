#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/type_traits.hpp>
#include <fs/utility/visitor.hpp>

#include <algorithm>
#include <utility>
#include <ostream>
#include <type_traits>

namespace fs::lang
{

namespace
{

integer make_int(int val)
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

void import_block::print(std::ostream& output_stream) const
{
	output_stream << keywords::rf::import_ << " \"" << path.value << '\"';

	if (is_optional)
		output_stream << ' ' << keywords::rf::optional;

	output_stream << "\n\n";
}

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

void item_filter::print(std::ostream& output_stream) const
{
	for (const block_variant& block_variant : blocks)
		std::visit([&](const auto& block) { block.print(output_stream); }, block_variant);
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
	if (!is_valid())
		return block_match_result::failure_invalid_block(visibility.origin, continuation.origin);

	std::vector<condition_match_result> match_results;
	for (const auto& cond : conditions.conditions)
		match_results.push_back(cond->test_item(itm, area_level));

	const bool is_successful = std::all_of(
		match_results.begin(),
		match_results.end(),
		[](const condition_match_result& result) { return result.is_successful(); });

	if (is_successful)
		return block_match_result::success(visibility.origin, continuation.origin, std::move(match_results));
	else
		return block_match_result::failure_mismatch(visibility.origin, continuation.origin, std::move(match_results));
}

item_filtering_result pass_item_through_filter(const item& itm, const item_filter& filter, int area_level)
{
	std::vector<block_match_result> match_history;
	// each item is expected to traverse half of the filter on average
	match_history.reserve(filter.blocks.size() / 2);

	item_style style = default_item_style(itm);

	for (const block_variant& block_variant : filter.blocks) {
		bool stop_filtering = std::visit(utility::visitor{
			[&](const import_block& block) {
				match_history.push_back(block_match_result::ignored_import_block(block.origin));
				return false;
			},
			[&](const item_filter_block& block) {
				block_match_result block_result = block.test_item(itm, area_level);

				bool stop = false;
				if (block_result.is_successful()) {
					style.override_with(block.actions);
					style.visibility = to_item_visibility_style(block.visibility);

					if (block.continuation.origin) {
						FS_ASSERT(is_valid(*block.continuation.origin));
					}
					else {
						stop = true;
					}
				}

				match_history.push_back(std::move(block_result));
				return stop;
			}
		}, block_variant);

		if (stop_filtering)
			break;
	}

	return item_filtering_result{style, std::move(match_history)};
}

} // namespace fs::lang
