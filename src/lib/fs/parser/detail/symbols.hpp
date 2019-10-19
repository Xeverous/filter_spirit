#pragma once

#include "fs/lang/types.hpp"
#include "fs/lang/keywords.hpp"

#include <boost/spirit/home/x3/string/symbols.hpp>

namespace fs::parser::detail::symbols
{

// symbols that denote filter's language constants
namespace x3 = boost::spirit::x3;

// ---- whitespace ----

// (nothing to symbolize)

// ---- fundamental tokens ----

// (nothing to symbolize)

// ---- version requirement ----

// (nothing to symbolize)

// ---- config ----

struct yes_no_ : x3::symbols<bool>
{
	yes_no_()
	{
		add
			(lang::keywords::yes, true)
			(lang::keywords::no, false)
		;
	}
};
const yes_no_ yes_no;

// ---- literal types ----

struct booleans_ : x3::symbols<bool>
{
	booleans_()
	{
		add
			(lang::keywords::true_, true)
			(lang::keywords::false_, false)
		;
	}

};
const booleans_ booleans;

struct rarities_ : x3::symbols<lang::rarity>
{
	rarities_()
	{
		add
			(lang::keywords::normal, lang::rarity::normal)
			(lang::keywords::magic,  lang::rarity::magic)
			(lang::keywords::rare,   lang::rarity::rare)
			(lang::keywords::unique, lang::rarity::unique)
		;
	}
};
const rarities_ rarities;

struct shapes_ : x3::symbols<lang::shape>
{
	shapes_()
	{
		add
			(lang::keywords::circle,   lang::shape::circle)
			(lang::keywords::diamond,  lang::shape::diamond)
			(lang::keywords::hexagon,  lang::shape::hexagon)
			(lang::keywords::square,   lang::shape::square)
			(lang::keywords::star,     lang::shape::star)
			(lang::keywords::triangle, lang::shape::triangle)
		;
	}
};
const shapes_ shapes;

struct suits_ : x3::symbols<lang::suit>
{
	suits_()
	{
		add
			(lang::keywords::red,    lang::suit::red)
			(lang::keywords::green,  lang::suit::green)
			(lang::keywords::blue,   lang::suit::blue)
			(lang::keywords::white,  lang::suit::white)
			(lang::keywords::brown,  lang::suit::brown)
			(lang::keywords::yellow, lang::suit::yellow)
		;
	}

};
const suits_ suits;

// ---- expressions ----

// (nothing to symbolize)

// ---- definitions ----

// (nothing to symbolize)

// ---- rules ----

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

struct comparison_condition_properties_ : x3::symbols<lang::comparison_condition_property>
{
	comparison_condition_properties_()
	{
		add
			(lang::keywords::item_level,     lang::comparison_condition_property::item_level)
			(lang::keywords::drop_level,     lang::comparison_condition_property::drop_level)
			(lang::keywords::quality,        lang::comparison_condition_property::quality)
			(lang::keywords::rarity,         lang::comparison_condition_property::rarity)
			(lang::keywords::sockets,        lang::comparison_condition_property::sockets)
			(lang::keywords::linked_sockets, lang::comparison_condition_property::links)
			(lang::keywords::height,         lang::comparison_condition_property::height)
			(lang::keywords::width,          lang::comparison_condition_property::width)
			(lang::keywords::stack_size,     lang::comparison_condition_property::stack_size)
			(lang::keywords::gem_level,      lang::comparison_condition_property::gem_level)
			(lang::keywords::map_tier,       lang::comparison_condition_property::map_tier)
		;
	}
};
const comparison_condition_properties_ comparison_condition_properties;

struct string_condition_properties_ : x3::symbols<lang::string_condition_property>
{
	string_condition_properties_()
	{
		add
			(lang::keywords::class_,           lang::string_condition_property::class_)
			(lang::keywords::base_type,        lang::string_condition_property::base_type)
			(lang::keywords::has_explicit_mod, lang::string_condition_property::has_explicit_mod)
			(lang::keywords::has_enchantment,  lang::string_condition_property::has_enchantment)
		;
	}
};
const string_condition_properties_ string_condition_properties;

struct boolean_condition_properties_ : x3::symbols<lang::boolean_condition_property>
{
	boolean_condition_properties_()
	{
		add
			(lang::keywords::identified,        lang::boolean_condition_property::identified)
			(lang::keywords::corrupted,         lang::boolean_condition_property::corrupted)
			(lang::keywords::elder_item,        lang::boolean_condition_property::elder_item)
			(lang::keywords::shaper_item,       lang::boolean_condition_property::shaper_item)
			(lang::keywords::fractured_item,    lang::boolean_condition_property::fractured_item)
			(lang::keywords::synthesised_item,  lang::boolean_condition_property::synthesised_item)
			(lang::keywords::any_enchantment,   lang::boolean_condition_property::any_enchantment)
			(lang::keywords::shaped_map,        lang::boolean_condition_property::shaped_map)
		;
	}
};
const boolean_condition_properties_ boolean_condition_properties;

struct unary_action_types_ : x3::symbols<lang::unary_action_type>
{
	unary_action_types_()
	{
		add
			(lang::keywords::set_border_color,           lang::unary_action_type::set_border_color)
			(lang::keywords::set_text_color,             lang::unary_action_type::set_text_color)
			(lang::keywords::set_background_color,       lang::unary_action_type::set_background_color)
			(lang::keywords::set_font_size,              lang::unary_action_type::set_font_size)
			(lang::keywords::set_alert_sound,            lang::unary_action_type::set_alert_sound)
			(lang::keywords::play_default_drop_sound,    lang::unary_action_type::play_default_drop_sound)
			(lang::keywords::set_minimap_icon,           lang::unary_action_type::set_minimap_icon)
			(lang::keywords::set_beam,                   lang::unary_action_type::set_beam)
		;
	}
};
const unary_action_types_ unary_action_types;

// ---- filter structure ----

struct visibility_literals_ : x3::symbols<bool>
{
	visibility_literals_()
	{
		add
			(lang::keywords::show, true)
			(lang::keywords::hide, false)
		;
	}

};
const visibility_literals_ visibility_literals;

}
