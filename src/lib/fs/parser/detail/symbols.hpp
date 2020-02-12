#pragma once

#include <fs/lang/condition_properties.hpp>
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

	struct rarities_ : x3::symbols<lang::rarity>
	{
		rarities_()
		{
			add
				(lang::keywords::rf::normal, lang::rarity::normal)
				(lang::keywords::rf::magic,  lang::rarity::magic)
				(lang::keywords::rf::rare,   lang::rarity::rare)
				(lang::keywords::rf::unique, lang::rarity::unique)
			;
		}
	};
	const rarities_ rarities;

	struct shapes_ : x3::symbols<lang::shape>
	{
		shapes_()
		{
			add
				(lang::keywords::rf::circle,   lang::shape::circle)
				(lang::keywords::rf::diamond,  lang::shape::diamond)
				(lang::keywords::rf::hexagon,  lang::shape::hexagon)
				(lang::keywords::rf::square,   lang::shape::square)
				(lang::keywords::rf::star,     lang::shape::star)
				(lang::keywords::rf::triangle, lang::shape::triangle)
			;
		}
	};
	const shapes_ shapes;

	struct suits_ : x3::symbols<lang::suit>
	{
		suits_()
		{
			add
				(lang::keywords::rf::red,    lang::suit::red)
				(lang::keywords::rf::green,  lang::suit::green)
				(lang::keywords::rf::blue,   lang::suit::blue)
				(lang::keywords::rf::white,  lang::suit::white)
				(lang::keywords::rf::brown,  lang::suit::brown)
				(lang::keywords::rf::yellow, lang::suit::yellow)
			;
		}

	};
	const suits_ suits;

	struct influences_ : x3::symbols<lang::influence>
	{
		influences_()
		{
			add
				(lang::keywords::rf::shaper,   lang::influence::shaper)
				(lang::keywords::rf::elder,    lang::influence::elder)
				(lang::keywords::rf::crusader, lang::influence::crusader)
				(lang::keywords::rf::redeemer, lang::influence::redeemer)
				(lang::keywords::rf::hunter,   lang::influence::hunter)
				(lang::keywords::rf::warlord,  lang::influence::warlord)
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
				(lang::keywords::rf::sockets,        lang::numeric_comparison_condition_property::sockets)
				(lang::keywords::rf::linked_sockets, lang::numeric_comparison_condition_property::links)
				(lang::keywords::rf::height,         lang::numeric_comparison_condition_property::height)
				(lang::keywords::rf::width,          lang::numeric_comparison_condition_property::width)
				(lang::keywords::rf::stack_size,     lang::numeric_comparison_condition_property::stack_size)
				(lang::keywords::rf::gem_level,      lang::numeric_comparison_condition_property::gem_level)
				(lang::keywords::rf::map_tier,       lang::numeric_comparison_condition_property::map_tier)
			;
		}
	};
	const numeric_comparison_condition_properties_ numeric_comparison_condition_properties;

	struct array_condition_properties_ : x3::symbols<lang::array_condition_property>
	{
		array_condition_properties_()
		{
			add
				(lang::keywords::rf::class_,           lang::array_condition_property::class_)
				(lang::keywords::rf::base_type,        lang::array_condition_property::base_type)
				(lang::keywords::rf::has_explicit_mod, lang::array_condition_property::has_explicit_mod)
				(lang::keywords::rf::has_enchantment,  lang::array_condition_property::has_enchantment)
				(lang::keywords::rf::prophecy,         lang::array_condition_property::prophecy)
				(lang::keywords::rf::has_influence,    lang::array_condition_property::has_influence)
			;
		}
	};
	const array_condition_properties_ array_condition_properties;

	struct string_array_condition_properties_ : x3::symbols<lang::string_array_condition_property>
	{
		string_array_condition_properties_()
		{
			add
				(lang::keywords::rf::class_,           lang::string_array_condition_property::class_)
				(lang::keywords::rf::base_type,        lang::string_array_condition_property::base_type)
				(lang::keywords::rf::has_explicit_mod, lang::string_array_condition_property::has_explicit_mod)
				(lang::keywords::rf::has_enchantment,  lang::string_array_condition_property::has_enchantment)
				(lang::keywords::rf::prophecy,         lang::string_array_condition_property::prophecy)
			;
		}
	};
	const string_array_condition_properties_ string_array_condition_properties;

	struct boolean_condition_properties_ : x3::symbols<lang::boolean_condition_property>
	{
		boolean_condition_properties_()
		{
			add
				(lang::keywords::rf::identified,        lang::boolean_condition_property::identified)
				(lang::keywords::rf::corrupted,         lang::boolean_condition_property::corrupted)
				(lang::keywords::rf::elder_item,        lang::boolean_condition_property::elder_item)
				(lang::keywords::rf::shaper_item,       lang::boolean_condition_property::shaper_item)
				(lang::keywords::rf::fractured_item,    lang::boolean_condition_property::fractured_item)
				(lang::keywords::rf::synthesised_item,  lang::boolean_condition_property::synthesised_item)
				(lang::keywords::rf::any_enchantment,   lang::boolean_condition_property::any_enchantment)
				(lang::keywords::rf::shaped_map,        lang::boolean_condition_property::shaped_map)
				(lang::keywords::rf::elder_map,         lang::boolean_condition_property::elder_map)
				(lang::keywords::rf::blighted_map,      lang::boolean_condition_property::blighted_map)
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
	struct comparison_condition_properties_ : x3::symbols<lang::comparison_condition_property>
	{
		comparison_condition_properties_()
		{
			add
				(lang::keywords::rf::item_level,     lang::comparison_condition_property::item_level)
				(lang::keywords::rf::drop_level,     lang::comparison_condition_property::drop_level)
				(lang::keywords::rf::quality,        lang::comparison_condition_property::quality)
				(lang::keywords::rf::rarity,         lang::comparison_condition_property::rarity)
				(lang::keywords::rf::sockets,        lang::comparison_condition_property::sockets)
				(lang::keywords::rf::linked_sockets, lang::comparison_condition_property::links)
				(lang::keywords::rf::height,         lang::comparison_condition_property::height)
				(lang::keywords::rf::width,          lang::comparison_condition_property::width)
				(lang::keywords::rf::stack_size,     lang::comparison_condition_property::stack_size)
				(lang::keywords::rf::gem_level,      lang::comparison_condition_property::gem_level)
				(lang::keywords::rf::map_tier,       lang::comparison_condition_property::map_tier)
			;
		}
	};
	const comparison_condition_properties_ comparison_condition_properties;

	struct array_condition_properties_ : x3::symbols<lang::array_condition_property>
	{
		array_condition_properties_()
		{
			add
				(lang::keywords::rf::class_,           lang::array_condition_property::class_)
				(lang::keywords::rf::base_type,        lang::array_condition_property::base_type)
				(lang::keywords::rf::has_explicit_mod, lang::array_condition_property::has_explicit_mod)
				(lang::keywords::rf::has_enchantment,  lang::array_condition_property::has_enchantment)
				(lang::keywords::rf::prophecy,         lang::array_condition_property::prophecy)
				(lang::keywords::rf::has_influence,    lang::array_condition_property::has_influence)
			;
		}
	};
	const array_condition_properties_ array_condition_properties;

	struct unary_action_types_ : x3::symbols<lang::unary_action_type>
	{
		unary_action_types_()
		{
			add
				(lang::keywords::rf::set_border_color,           lang::unary_action_type::set_border_color)
				(lang::keywords::rf::set_text_color,             lang::unary_action_type::set_text_color)
				(lang::keywords::rf::set_background_color,       lang::unary_action_type::set_background_color)
				(lang::keywords::rf::set_font_size,              lang::unary_action_type::set_font_size)
				(lang::keywords::sf::set_alert_sound,            lang::unary_action_type::set_alert_sound)
				(lang::keywords::sf::play_default_drop_sound,    lang::unary_action_type::play_default_drop_sound)
				(lang::keywords::sf::set_minimap_icon,           lang::unary_action_type::set_minimap_icon)
				(lang::keywords::sf::set_beam,                   lang::unary_action_type::set_beam)
			;
		}
	};
	const unary_action_types_ unary_action_types;
} // namespace sf

} // namespace fs::parser::detail::symbols
