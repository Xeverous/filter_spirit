#pragma once

#include <fs/lang/keywords.hpp>
#include <fs/utility/exceptions.hpp>
#include <fs/utility/better_enum.hpp>

#include <string_view>
#include <optional>

namespace fs::lang {

// ---- conditions ----

enum class official_condition_property
{
	// boolean conditions
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
	scourged,
	uber_blighted_map,
	has_implicit_mod,
	has_crucible_passive_tree,
	alternate_quality,
	transfigured_gem,

	// comparison with influence
	has_influence,

	// comparison with rarities
	rarity,

	// comparison with integers
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
	base_armour,
	base_evasion,
	base_energy_shield,
	base_ward,
	base_defence_percentile,
	has_searing_exarch_implicit,
	has_eater_of_worlds_implicit,

	// comparison with an array of strings
	class_,
	base_type,
	enchantment_passive_node,
	archnemesis_mod,

	// comparison (+ optional integer) with an array of strings
	has_explicit_mod,
	has_enchantment,

	// socket conditions
	sockets,
	socket_group
};

inline std::string_view to_keyword(official_condition_property property)
{
	switch (property) {
		case official_condition_property::identified:
			return keywords::rf::identified;
		case official_condition_property::corrupted:
			return keywords::rf::corrupted;
		case official_condition_property::mirrored:
			return keywords::rf::mirrored;
		case official_condition_property::elder_item:
			return keywords::rf::elder_item;
		case official_condition_property::shaper_item:
			return keywords::rf::shaper_item;
		case official_condition_property::fractured_item:
			return keywords::rf::fractured_item;
		case official_condition_property::synthesised_item:
			return keywords::rf::synthesised_item;
		case official_condition_property::any_enchantment:
			return keywords::rf::any_enchantment;
		case official_condition_property::shaped_map:
			return keywords::rf::shaped_map;
		case official_condition_property::elder_map:
			return keywords::rf::elder_map;
		case official_condition_property::blighted_map:
			return keywords::rf::blighted_map;
		case official_condition_property::replica:
			return keywords::rf::replica;
		case official_condition_property::scourged:
			return keywords::rf::scourged;
		case official_condition_property::uber_blighted_map:
			return keywords::rf::uber_blighted_map;
		case official_condition_property::has_implicit_mod:
			return keywords::rf::has_implicit_mod;
		case official_condition_property::has_crucible_passive_tree:
			return keywords::rf::has_crucible_passive_tree;
		case official_condition_property::alternate_quality:
			return keywords::rf::alternate_quality;
		case official_condition_property::transfigured_gem:
			return keywords::rf::transfigured_gem;
		case official_condition_property::has_influence:
			return keywords::rf::has_influence;
		case official_condition_property::rarity:
			return keywords::rf::rarity;
		case official_condition_property::item_level:
			return keywords::rf::item_level;
		case official_condition_property::drop_level:
			return keywords::rf::drop_level;
		case official_condition_property::quality:
			return keywords::rf::quality;
		case official_condition_property::linked_sockets:
			return keywords::rf::linked_sockets;
		case official_condition_property::height:
			return keywords::rf::height;
		case official_condition_property::width:
			return keywords::rf::width;
		case official_condition_property::stack_size:
			return keywords::rf::stack_size;
		case official_condition_property::gem_level:
			return keywords::rf::gem_level;
		case official_condition_property::map_tier:
			return keywords::rf::map_tier;
		case official_condition_property::area_level:
			return keywords::rf::area_level;
		case official_condition_property::corrupted_mods:
			return keywords::rf::corrupted_mods;
		case official_condition_property::enchantment_passive_num:
			return keywords::rf::enchantment_passive_num;
		case official_condition_property::base_defence_percentile:
			return keywords::rf::base_defence_percentile;
		case official_condition_property::base_armour:
			return keywords::rf::base_armour;
		case official_condition_property::base_evasion:
			return keywords::rf::base_evasion;
		case official_condition_property::base_energy_shield:
			return keywords::rf::base_energy_shield;
		case official_condition_property::base_ward:
			return keywords::rf::base_ward;
		case official_condition_property::has_searing_exarch_implicit:
			return keywords::rf::has_searing_exarch_implicit;
		case official_condition_property::has_eater_of_worlds_implicit:
			return keywords::rf::has_eater_of_worlds_implicit;
		case official_condition_property::class_:
			return keywords::rf::class_;
		case official_condition_property::base_type:
			return keywords::rf::base_type;
		case official_condition_property::enchantment_passive_node:
			return keywords::rf::enchantment_passive_node;
		case official_condition_property::archnemesis_mod:
			return keywords::rf::archnemesis_mod;
		case official_condition_property::has_explicit_mod:
			return keywords::rf::has_explicit_mod;
		case official_condition_property::has_enchantment:
			return keywords::rf::has_enchantment;
		case official_condition_property::sockets:
			return keywords::rf::sockets;
		case official_condition_property::socket_group:
			return keywords::rf::socket_group;
	}

	throw unhandled_switch_case(__FILE__, __LINE__, __func__);
}

enum class comparison_type
{
	less,
	less_equal,
	equal, // both nothing and =
	exact_match, // ==
	greater,
	greater_equal,
	not_equal // both != and !
};

inline std::string_view to_string_view(comparison_type cmp)
{
	switch (cmp) {
		case comparison_type::less:
			return "<";
		case comparison_type::less_equal:
			return "<=";
		case comparison_type::equal:
			return "";
		case comparison_type::exact_match:
			return "==";
		case comparison_type::greater:
			return ">";
		case comparison_type::greater_equal:
			return ">=";
		case comparison_type::not_equal:
			return "!=";
	}

	throw unhandled_switch_case(__FILE__, __LINE__, __func__);
}

enum class equality_comparison_type { equal, exact_match, not_equal };

inline comparison_type to_comparison_type(equality_comparison_type cmp)
{
	switch (cmp) {
		case equality_comparison_type::equal:
			return comparison_type::equal;
		case equality_comparison_type::exact_match:
			return comparison_type::exact_match;
		case equality_comparison_type::not_equal:
			return comparison_type::not_equal;
	}

	throw unhandled_switch_case(__FILE__, __LINE__, __func__);
}

inline std::optional<equality_comparison_type> to_equality_comparison_type(comparison_type cmp)
{
	switch (cmp) {
		case comparison_type::equal:
			return equality_comparison_type::equal;
		case comparison_type::exact_match:
			return equality_comparison_type::exact_match;
		case comparison_type::not_equal:
			return equality_comparison_type::not_equal;
		default:
			return std::nullopt;
	}
}

// ---- actions ----

enum class official_action_property
{
	set_text_color,
	set_border_color,
	set_background_color,
	set_font_size,
	play_effect,
	minimap_icon,
	play_alert_sound,
	play_alert_sound_positional,
	custom_alert_sound,
	custom_alert_sound_optional,
	disable_drop_sound,
	enable_drop_sound,
	disable_drop_sound_if_alert_sound,
	enable_drop_sound_if_alert_sound
};

// ---- extensions ----

// TODO rename Autogen names to be easier to understand
// Then replace BETTER_ENUM with manual implementation
BETTER_ENUM(autogen_category, int,
	currency,
	fragments,
	delirium_orbs,
	cards,
	essences,
	fossils,
	resonators,
	scarabs,
	incubators,
	oils,
	vials,

	gems,

	bases,

	uniques_eq_unambiguous,
	uniques_eq_ambiguous,
	uniques_flasks_unambiguous,
	uniques_flasks_ambiguous,
	uniques_jewels_unambiguous,
	uniques_jewels_ambiguous,
	uniques_maps_unambiguous,
	uniques_maps_ambiguous

	// not supported (yet)
	// invitations
)

inline bool is_uniques_category(autogen_category cat) noexcept
{
	return
		   cat == +autogen_category::uniques_eq_unambiguous
		|| cat == +autogen_category::uniques_eq_ambiguous
		|| cat == +autogen_category::uniques_flasks_unambiguous
		|| cat == +autogen_category::uniques_flasks_ambiguous
		|| cat == +autogen_category::uniques_jewels_unambiguous
		|| cat == +autogen_category::uniques_jewels_ambiguous
		|| cat == +autogen_category::uniques_maps_unambiguous
		|| cat == +autogen_category::uniques_maps_ambiguous;
}

} // namespace fs::lang
