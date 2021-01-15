#pragma once

#include <fs/lang/condition_properties.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/lang/queries.hpp>

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
				("=",  lang::comparison_type::equal_soft)
				("==", lang::comparison_type::equal_hard)
				(">",  lang::comparison_type::greater)
				(">=", lang::comparison_type::greater_equal)
			;
		}
	};
	const comparison_operators_ comparison_operators;

	struct numeric_comparison_condition_properties_ : x3::symbols<lang::numeric_comparison_condition_property>
	{
		numeric_comparison_condition_properties_()
		{
			add
				(lang::keywords::rf::item_level,     lang::numeric_comparison_condition_property::item_level)
				(lang::keywords::rf::drop_level,     lang::numeric_comparison_condition_property::drop_level)
				(lang::keywords::rf::quality,        lang::numeric_comparison_condition_property::quality)
				(lang::keywords::rf::linked_sockets, lang::numeric_comparison_condition_property::linked_sockets)
				(lang::keywords::rf::height,         lang::numeric_comparison_condition_property::height)
				(lang::keywords::rf::width,          lang::numeric_comparison_condition_property::width)
				(lang::keywords::rf::stack_size,     lang::numeric_comparison_condition_property::stack_size)
				(lang::keywords::rf::gem_level,      lang::numeric_comparison_condition_property::gem_level)
				(lang::keywords::rf::map_tier,       lang::numeric_comparison_condition_property::map_tier)
				(lang::keywords::rf::area_level,     lang::numeric_comparison_condition_property::area_level)
				(lang::keywords::rf::corrupted_mods, lang::numeric_comparison_condition_property::corrupted_mods)
				(lang::keywords::rf::enchantment_passive_num, lang::numeric_comparison_condition_property::enchantment_passive_num)
			;
		}
	};
	const numeric_comparison_condition_properties_ numeric_comparison_condition_properties;

	struct string_array_condition_properties_ : x3::symbols<lang::string_array_condition_property>
	{
		string_array_condition_properties_()
		{
			add
				(lang::keywords::rf::class_,                   lang::string_array_condition_property::class_)
				(lang::keywords::rf::base_type,                lang::string_array_condition_property::base_type)
				(lang::keywords::rf::prophecy,                 lang::string_array_condition_property::prophecy)
				(lang::keywords::rf::enchantment_passive_node, lang::string_array_condition_property::enchantment_passive_node)
			;
		}
	};
	const string_array_condition_properties_ string_array_condition_properties;

	struct ranged_string_array_condition_properties_ : x3::symbols<lang::ranged_string_array_condition_property>
	{
		ranged_string_array_condition_properties_()
		{
			add
				(lang::keywords::rf::has_explicit_mod, lang::ranged_string_array_condition_property::has_explicit_mod)
				(lang::keywords::rf::has_enchantment,  lang::ranged_string_array_condition_property::has_enchantment)
			;
		}
	};
	const ranged_string_array_condition_properties_ ranged_string_array_condition_properties;

	struct socket_spec_condition_properties_ : x3::symbols<bool>
	{
		socket_spec_condition_properties_()
		{
			add
				(lang::keywords::rf::sockets,      false)
				(lang::keywords::rf::socket_group, true)
			;
		}
	};
	const socket_spec_condition_properties_ socket_spec_condition_properties;

	struct boolean_condition_properties_ : x3::symbols<lang::boolean_condition_property>
	{
		boolean_condition_properties_()
		{
			add
				(lang::keywords::rf::identified,        lang::boolean_condition_property::identified)
				(lang::keywords::rf::corrupted,         lang::boolean_condition_property::corrupted)
				(lang::keywords::rf::mirrored,          lang::boolean_condition_property::mirrored)
				(lang::keywords::rf::elder_item,        lang::boolean_condition_property::elder_item)
				(lang::keywords::rf::shaper_item,       lang::boolean_condition_property::shaper_item)
				(lang::keywords::rf::fractured_item,    lang::boolean_condition_property::fractured_item)
				(lang::keywords::rf::synthesised_item,  lang::boolean_condition_property::synthesised_item)
				(lang::keywords::rf::any_enchantment,   lang::boolean_condition_property::any_enchantment)
				(lang::keywords::rf::shaped_map,        lang::boolean_condition_property::shaped_map)
				(lang::keywords::rf::elder_map,         lang::boolean_condition_property::elder_map)
				(lang::keywords::rf::blighted_map,      lang::boolean_condition_property::blighted_map)
				(lang::keywords::rf::replica,           lang::boolean_condition_property::replica)
				(lang::keywords::rf::alternate_quality, lang::boolean_condition_property::alternate_quality)
			;
		}
	};
	const boolean_condition_properties_ boolean_condition_properties;

	// ---- actions ----

	struct color_actions_ : x3::symbols<lang::color_action_type>
	{
		color_actions_()
		{
			add
				(lang::keywords::rf::set_border_color,     lang::color_action_type::set_border_color)
				(lang::keywords::rf::set_text_color,       lang::color_action_type::set_text_color)
				(lang::keywords::rf::set_background_color, lang::color_action_type::set_background_color)
			;
		}
	};
	const color_actions_ color_actions;

	struct play_alert_sound_actions_ : x3::symbols<bool>
	{
		play_alert_sound_actions_()
		{
			add
				(lang::keywords::rf::play_alert_sound,            false)
				(lang::keywords::rf::play_alert_sound_positional, true)
			;
		}
	};
	const play_alert_sound_actions_ play_alert_sound_actions;

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

	struct gem_quality_types_ : x3::symbols<lang::gem_quality_type_type>
	{
		gem_quality_types_()
		{
			add
				(lang::keywords::rf::superior,   lang::gem_quality_type_type::superior)
				(lang::keywords::rf::divergent,  lang::gem_quality_type_type::divergent)
				(lang::keywords::rf::anomalous,  lang::gem_quality_type_type::anomalous)
				(lang::keywords::rf::phantasmal, lang::gem_quality_type_type::phantasmal)
			;
		}
	};
	const gem_quality_types_ gem_quality_types;

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

// symbols used exclusively in spirit filter template language
namespace sf
{
	struct item_categories_ : x3::symbols<lang::item_category>
	{
		item_categories_()
		{
			add
				(lang::keywords::sf::cards,           fs::lang::item_category::cards)
				(lang::keywords::sf::prophecies,      fs::lang::item_category::prophecies)
				(lang::keywords::sf::essences,        fs::lang::item_category::essences)
				(lang::keywords::sf::fossils,         fs::lang::item_category::fossils)
				(lang::keywords::sf::resonators,      fs::lang::item_category::resonators)
				(lang::keywords::sf::scarabs,         fs::lang::item_category::scarabs)
				(lang::keywords::sf::incubators,      fs::lang::item_category::incubators)
				(lang::keywords::sf::oils,            fs::lang::item_category::oils)
				(lang::keywords::sf::catalysts,       fs::lang::item_category::catalysts)
				(lang::keywords::sf::uniques_eq_unambiguous,    fs::lang::item_category::uniques_eq_unambiguous)
				(lang::keywords::sf::uniques_eq_ambiguous,      fs::lang::item_category::uniques_eq_ambiguous)
				(lang::keywords::sf::uniques_flasks_unambiguous, fs::lang::item_category::uniques_flasks_unambiguous)
				(lang::keywords::sf::uniques_flasks_ambiguous,   fs::lang::item_category::uniques_flasks_ambiguous)
				(lang::keywords::sf::uniques_jewels_unambiguous, fs::lang::item_category::uniques_jewels_unambiguous)
				(lang::keywords::sf::uniques_jewels_ambiguous,   fs::lang::item_category::uniques_jewels_ambiguous)
				(lang::keywords::sf::uniques_maps_unambiguous,   fs::lang::item_category::uniques_maps_unambiguous)
				(lang::keywords::sf::uniques_maps_ambiguous,     fs::lang::item_category::uniques_maps_ambiguous)

				// not supported (yet)
				// (lang::keywords::sf::enchants_helmet, fs::lang::item_category::enchants_helmet)
			;
		}
	};
	const item_categories_ item_categories;
} // namespace sf

} // namespace fs::parser::detail::symbols
