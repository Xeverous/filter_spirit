#pragma once

#include <fs/lang/condition_set.hpp>
#include <fs/lang/action_set.hpp>
#include <fs/lang/item.hpp>

#include <iosfwd>
#include <optional>
#include <vector>

namespace fs::lang
{

struct item_visibility
{
	item_visibility(bool show, position_tag origin)
	: show(show), origin(origin)
	{
	}

	bool show;
	position_tag origin;
};

struct block_continuation
{
	std::optional<position_tag> origin;
};

struct item_filter_block
{
	item_filter_block(item_visibility visibility)
	: visibility(visibility)
	{
	}

	item_filter_block(
		item_visibility visibility,
		condition_set conditions,
		action_set actions,
		block_continuation continuation)
	: visibility(visibility)
	, conditions(conditions)
	, actions(actions)
	, continuation(continuation)
	{
	}

	void generate(std::ostream& output_stream) const;

	item_visibility visibility;
	condition_set conditions;
	action_set actions;
	block_continuation continuation;
};

struct autogen_extension
{
	fractional_range_condition price_range;
	autogen_condition condition;
};

struct spirit_item_filter_block
{
	item_filter_block block;

	// spirit filter extensions
	std::optional<autogen_extension> autogen;
};

/**
 * @class a core type representing item filter
 *
 * @details This is a result of the parsers and a type
 * used in various debugging functionalities
 */
struct item_filter
{
	std::vector<item_filter_block> blocks;
};

struct spirit_item_filter
{
	std::vector<spirit_item_filter_block> blocks;
};

// all of the styles applied to an item after filtering
struct item_style
{
	item_style();

	void override_with(const action_set& actions);

	item_visibility visibility;

	std::optional<color_action> border_color;
	color_action text_color;
	color_action background_color;
	font_size_action font_size;
	std::optional<alert_sound_action> play_alert_sound;
	std::optional<switch_drop_sound_action> switch_drop_sound;
	std::optional<minimap_icon_action> minimap_icon;
	std::optional<play_effect_action> play_effect;
};

struct block_match_result
{
	std::optional<lang::position_tag> continue_origin;
	condition_set_match_result condition_set_result;
};

struct item_filtering_result
{
	// final style applied to the item
	item_style style;

	// each element represents a match attempt against next filter block
	std::vector<block_match_result> match_history;
};

item_filtering_result pass_item_through_filter(const item& itm, const item_filter& filter, int area_level);

}
