#pragma once

namespace fs::lang
{

// < <= = > >=
enum class comparison_type
{
	less,
	less_equal,
	equal,
	greater,
	greater_equal
};

// all conditions which use comparison against integers
// (currently comparison conditions except rarity)
enum class numeric_comparison_condition_property
{
	item_level,
	drop_level,
	quality,
	sockets,
	links,
	height,
	width,
	stack_size,
	gem_level,
	map_tier
};

// all conditions which work with array of strings
// (basically all array conditions except influence)
enum class string_array_condition_property
{
	class_,
	base_type,
	has_explicit_mod,
	has_enchantment,
	prophecy,
};

// all conditions which are boolean tests
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

enum class socket_spec_condition_property
{
	sockets,
	socket_group
};

} // namespace fs::lang
