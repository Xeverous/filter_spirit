#pragma once

namespace fs::lang
{

enum class comparison_type
{
	less,
	less_equal,
	equal,
	greater,
	greater_equal
};

enum class comparison_condition_property
{
	item_level,
	drop_level,
	quality,
	rarity,
	sockets,
	links,
	height,
	width,
	stack_size,
	gem_level,
	map_tier
};

enum class string_condition_property
{
	class_,
	base_type,
	has_explicit_mod,
	has_enchantment,
	prophecy
};

enum class boolean_condition_property
{
	identified,
	corrupted,
	elder_item,
	shaper_item,
	fractured_item,
	synthesised_item,
	any_enchantment,
	shaped_map,
	elder_map,
	blighted_map
};

} // namespace fs::lang
