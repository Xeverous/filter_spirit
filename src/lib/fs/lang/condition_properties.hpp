#pragma once

namespace fs::lang
{

enum class comparison_type
{
	less,
	less_equal,
	equal_soft,
	equal_hard,
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
	linked_sockets,
	height,
	width,
	stack_size,
	gem_level,
	map_tier,
	area_level,
	corrupted_mods,
	enchantment_passive_num,
	base_defence_percentile,
	base_armour,
	base_evasion,
	base_energy_shield,
	base_ward,
	has_searing_exarch_implicit,
	has_eater_of_worlds_implicit
};

// conditions which work with array of strings
// (basically all array conditions except influence)
enum class string_array_condition_property
{
	class_,
	base_type,
	enchantment_passive_node,
	archnemesis_mod
};

// conditions which work with array of strings AND optionally a comparison
enum class ranged_string_array_condition_property
{
	has_explicit_mod,
	has_enchantment
};

// all conditions which are boolean tests
enum class boolean_condition_property
{
	identified,
	corrupted,
	mirrored,
	elder_item,
	shaper_item,
	fractured_item,
	synthesised_item,
	any_enchantment,
	shaped_map,
	elder_map,
	blighted_map,
	replica,
	alternate_quality,
	scourged,
	uber_blighted_map
};

} // namespace fs::lang
