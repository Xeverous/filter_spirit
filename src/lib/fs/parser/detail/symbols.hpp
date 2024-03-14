#pragma once

#include <fs/lang/enum_types.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/keywords.hpp>

#include <boost/spirit/home/x3/string/symbols.hpp>

// symbols that denote filter's language constants
namespace fs::parser::detail::symbols
{

namespace x3 = boost::spirit::x3;

// symbols used in real filter grammar (and potentially in spirit filter templates)
namespace rf
{
	// ---- whitespace ----

	// (nothing to symbolize)

	// ---- fundamental tokens ----

	// (nothing to symbolize)

	// ---- literal types ----

	struct booleans_ : x3::symbols<bool>
	{
		booleans_()
		{
			add
				(lang::keywords::rf::true_, true)
				(lang::keywords::rf::false_, false)
			;
		}

	};
	const booleans_ booleans;

	struct rarities_ : x3::symbols<lang::rarity_type>
	{
		rarities_()
		{
			add
				(lang::keywords::rf::normal, lang::rarity_type::normal)
				(lang::keywords::rf::magic,  lang::rarity_type::magic)
				(lang::keywords::rf::rare,   lang::rarity_type::rare)
				(lang::keywords::rf::unique, lang::rarity_type::unique)
			;
		}
	};
	const rarities_ rarities;

	struct shapes_ : x3::symbols<lang::shape_type>
	{
		shapes_()
		{
			add
				(lang::keywords::rf::circle,   lang::shape_type::circle)
				(lang::keywords::rf::diamond,  lang::shape_type::diamond)
				(lang::keywords::rf::hexagon,  lang::shape_type::hexagon)
				(lang::keywords::rf::square,   lang::shape_type::square)
				(lang::keywords::rf::star,     lang::shape_type::star)
				(lang::keywords::rf::triangle, lang::shape_type::triangle)
				(lang::keywords::rf::cross,    lang::shape_type::cross)
				(lang::keywords::rf::moon,     lang::shape_type::moon)
				(lang::keywords::rf::raindrop, lang::shape_type::raindrop)
				(lang::keywords::rf::kite,     lang::shape_type::kite)
				(lang::keywords::rf::pentagon, lang::shape_type::pentagon)
				(lang::keywords::rf::upside_down_house, lang::shape_type::upside_down_house)
			;
		}
	};
	const shapes_ shapes;

	struct suits_ : x3::symbols<lang::suit_type>
	{
		suits_()
		{
			add
				(lang::keywords::rf::red,    lang::suit_type::red)
				(lang::keywords::rf::green,  lang::suit_type::green)
				(lang::keywords::rf::blue,   lang::suit_type::blue)
				(lang::keywords::rf::white,  lang::suit_type::white)
				(lang::keywords::rf::brown,  lang::suit_type::brown)
				(lang::keywords::rf::yellow, lang::suit_type::yellow)
				(lang::keywords::rf::cyan,   lang::suit_type::cyan)
				(lang::keywords::rf::grey,   lang::suit_type::grey)
				(lang::keywords::rf::orange, lang::suit_type::orange)
				(lang::keywords::rf::pink,   lang::suit_type::pink)
				(lang::keywords::rf::purple, lang::suit_type::purple)
			;
		}

	};
	const suits_ suits;

	struct influences_ : x3::symbols<lang::influence_type>
	{
		influences_()
		{
			add
				(lang::keywords::rf::shaper,   lang::influence_type::shaper)
				(lang::keywords::rf::elder,    lang::influence_type::elder)
				(lang::keywords::rf::crusader, lang::influence_type::crusader)
				(lang::keywords::rf::redeemer, lang::influence_type::redeemer)
				(lang::keywords::rf::hunter,   lang::influence_type::hunter)
				(lang::keywords::rf::warlord,  lang::influence_type::warlord)
			;
		}
	};
	const influences_ influences;

	// ---- expressions ----

	// (nothing to symbolize)

	// ---- definitions ----

	// (nothing to symbolize)

	// ---- conditions ----

	struct comparison_operators_ : x3::symbols<lang::comparison_type>
	{
		comparison_operators_()
		{
			add
				("<",  lang::comparison_type::less)
				("<=", lang::comparison_type::less_equal)
				("=",  lang::comparison_type::equal)
				("==", lang::comparison_type::exact_match)
				(">",  lang::comparison_type::greater)
				(">=", lang::comparison_type::greater_equal)
				("!=", lang::comparison_type::not_equal)
				("!",  lang::comparison_type::not_equal)
			;
		}
	};
	const comparison_operators_ comparison_operators;

	struct official_condition_properties_ : x3::symbols<lang::official_condition_property>
	{
		official_condition_properties_()
		{
			using property = lang::official_condition_property;

			add
				// boolean
				(lang::keywords::rf::identified,                property::identified)
				(lang::keywords::rf::corrupted,                 property::corrupted)
				(lang::keywords::rf::mirrored,                  property::mirrored)
				(lang::keywords::rf::elder_item,                property::elder_item)
				(lang::keywords::rf::shaper_item,               property::shaper_item)
				(lang::keywords::rf::fractured_item,            property::fractured_item)
				(lang::keywords::rf::synthesised_item,          property::synthesised_item)
				(lang::keywords::rf::any_enchantment,           property::any_enchantment)
				(lang::keywords::rf::shaped_map,                property::shaped_map)
				(lang::keywords::rf::elder_map,                 property::elder_map)
				(lang::keywords::rf::blighted_map,              property::blighted_map)
				(lang::keywords::rf::replica,                   property::replica)
				(lang::keywords::rf::scourged,                  property::scourged)
				(lang::keywords::rf::uber_blighted_map,         property::uber_blighted_map)
				(lang::keywords::rf::has_implicit_mod,          property::has_implicit_mod)
				(lang::keywords::rf::has_crucible_passive_tree, property::has_crucible_passive_tree)
				(lang::keywords::rf::alternate_quality,         property::alternate_quality)
				(lang::keywords::rf::transfigured_gem,          property::transfigured_gem)

				// influence
				(lang::keywords::rf::has_influence,             property::has_influence)

				// rarity
				(lang::keywords::rf::rarity,                    property::rarity)

				// integer
				(lang::keywords::rf::item_level,                   property::item_level)
				(lang::keywords::rf::drop_level,                   property::drop_level)
				(lang::keywords::rf::quality,                      property::quality)
				(lang::keywords::rf::linked_sockets,               property::linked_sockets)
				(lang::keywords::rf::height,                       property::height)
				(lang::keywords::rf::width,                        property::width)
				(lang::keywords::rf::stack_size,                   property::stack_size)
				(lang::keywords::rf::gem_level,                    property::gem_level)
				(lang::keywords::rf::map_tier,                     property::map_tier)
				(lang::keywords::rf::area_level,                   property::area_level)
				(lang::keywords::rf::corrupted_mods,               property::corrupted_mods)
				(lang::keywords::rf::enchantment_passive_num,      property::enchantment_passive_num)
				(lang::keywords::rf::base_defence_percentile,      property::base_defence_percentile)
				(lang::keywords::rf::base_armour,                  property::base_armour)
				(lang::keywords::rf::base_evasion,                 property::base_evasion)
				(lang::keywords::rf::base_energy_shield,           property::base_energy_shield)
				(lang::keywords::rf::base_ward,                    property::base_ward)
				(lang::keywords::rf::has_searing_exarch_implicit,  property::has_searing_exarch_implicit)
				(lang::keywords::rf::has_eater_of_worlds_implicit, property::has_eater_of_worlds_implicit)

				// array of strings
				(lang::keywords::rf::class_,                   property::class_)
				(lang::keywords::rf::base_type,                property::base_type)
				(lang::keywords::rf::enchantment_passive_node, property::enchantment_passive_node)
				(lang::keywords::rf::archnemesis_mod,          property::archnemesis_mod)

				// counted array of strings
				(lang::keywords::rf::has_explicit_mod, property::has_explicit_mod)
				(lang::keywords::rf::has_enchantment,  property::has_enchantment)

				// sockets
				(lang::keywords::rf::sockets,      property::sockets)
				(lang::keywords::rf::socket_group, property::socket_group)
			;
		}
	};
	const official_condition_properties_ official_condition_properties;

	// ---- actions ----

	struct official_action_properties_ : x3::symbols<lang::official_action_property>
	{
		official_action_properties_()
		{
			using property = lang::official_action_property;

			add
				(lang::keywords::rf::set_text_color,                    property::set_text_color)
				(lang::keywords::rf::set_background_color,              property::set_background_color)
				(lang::keywords::rf::set_border_color,                  property::set_border_color)
				(lang::keywords::rf::set_font_size,                     property::set_font_size)
				(lang::keywords::rf::play_effect,                       property::play_effect)
				(lang::keywords::rf::minimap_icon,                      property::minimap_icon)
				(lang::keywords::rf::play_alert_sound,                  property::play_alert_sound)
				(lang::keywords::rf::play_alert_sound_positional,       property::play_alert_sound_positional)
				(lang::keywords::rf::custom_alert_sound,                property::custom_alert_sound)
				(lang::keywords::rf::custom_alert_sound_optional,       property::custom_alert_sound_optional)
				(lang::keywords::rf::disable_drop_sound,                property::disable_drop_sound)
				(lang::keywords::rf::enable_drop_sound,                 property::enable_drop_sound)
				(lang::keywords::rf::disable_drop_sound_if_alert_sound, property::disable_drop_sound_if_alert_sound)
				(lang::keywords::rf::enable_drop_sound_if_alert_sound,  property::enable_drop_sound_if_alert_sound)
			;
		}
	};
	const official_action_properties_ official_action_properties;

	struct shaper_voice_lines_ : x3::symbols<lang::shaper_voice_line_type>
	{
		shaper_voice_lines_()
		{
			add
				(lang::keywords::rf::sh_mirror,  lang::shaper_voice_line_type::mirror)
				(lang::keywords::rf::sh_exalted, lang::shaper_voice_line_type::exalted)
				(lang::keywords::rf::sh_divine,  lang::shaper_voice_line_type::divine)
				(lang::keywords::rf::sh_general, lang::shaper_voice_line_type::general)
				(lang::keywords::rf::sh_regal,   lang::shaper_voice_line_type::regal)
				(lang::keywords::rf::sh_chaos,   lang::shaper_voice_line_type::chaos)
				(lang::keywords::rf::sh_fusing,  lang::shaper_voice_line_type::fusing)
				(lang::keywords::rf::sh_alchemy, lang::shaper_voice_line_type::alchemy)
				(lang::keywords::rf::sh_vaal,    lang::shaper_voice_line_type::vaal)
				(lang::keywords::rf::sh_blessed, lang::shaper_voice_line_type::blessed)
			;
		}
	};
	const shaper_voice_lines_ shaper_voice_lines;

	// ---- filter structure ----

	struct visibility_literals_ : x3::symbols<bool>
	{
		visibility_literals_()
		{
			add
				(lang::keywords::rf::show, true)
				(lang::keywords::rf::hide, false)
			;
		}

	};
	const visibility_literals_ visibility_literals;
} // namespace rf

} // namespace fs::parser::detail::symbols
