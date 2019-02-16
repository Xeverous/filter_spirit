#pragma once
#include "parser/ast.hpp"
#include "lang/types.hpp"
#include "lang/constants.hpp"
#include <boost/spirit/home/x3.hpp>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;

// symbols that denote filter's language constants

struct yes_no_ : x3::symbols<bool>
{
	yes_no_()
	{
		add
			(lang::constants::keywords::yes, true)
			(lang::constants::keywords::no, false)
		;
	}
};
const yes_no_ yes_no;

struct booleans_ : x3::symbols<bool>
{
	booleans_()
	{
		add
			(lang::constants::keywords::true_, true)
			(lang::constants::keywords::false_, false)
		;
	}

};
const booleans_ booleans;

struct rarities_ : x3::symbols<lang::rarity>
{
	rarities_()
	{
		add
			(lang::constants::keywords::normal, lang::rarity::normal)
			(lang::constants::keywords::magic,  lang::rarity::magic)
			(lang::constants::keywords::rare,   lang::rarity::rare)
			(lang::constants::keywords::unique, lang::rarity::unique)
		;
	}
};
const rarities_ rarities;

struct shapes_ : x3::symbols<lang::shape>
{
	shapes_()
	{
		add
			(lang::constants::keywords::circle,   lang::shape::circle)
			(lang::constants::keywords::diamond,  lang::shape::diamond)
			(lang::constants::keywords::hexagon,  lang::shape::hexagon)
			(lang::constants::keywords::square,   lang::shape::square)
			(lang::constants::keywords::star,     lang::shape::star)
			(lang::constants::keywords::triangle, lang::shape::triangle)
		;
	}
};
const shapes_ shapes;

struct suits_ : x3::symbols<lang::suit>
{
	suits_()
	{
		add
			(lang::constants::keywords::red,    lang::suit::red)
			(lang::constants::keywords::green,  lang::suit::green)
			(lang::constants::keywords::blue,   lang::suit::blue)
			(lang::constants::keywords::white,  lang::suit::white)
			(lang::constants::keywords::brown,  lang::suit::brown)
			(lang::constants::keywords::yellow, lang::suit::yellow)
		;
	}

};
const suits_ suits;

struct comparison_operators_ : x3::symbols<lang::comparison_type>
{
	comparison_operators_()
	{
		add
			("<",  lang::comparison_type::less)
			("<=", lang::comparison_type::less_equal)
			("==", lang::comparison_type::equal)
			(">",  lang::comparison_type::greater)
			(">=", lang::comparison_type::greater_equal)
		;
	}
};
const comparison_operators_ comparison_operators;

struct action_types_ : x3::symbols<lang::action_type>
{
	action_types_()
	{
		add
			(lang::constants::keywords::set_border_color,     lang::action_type::set_border_color)
			(lang::constants::keywords::set_text_color,       lang::action_type::set_text_color)
			(lang::constants::keywords::set_background_color, lang::action_type::set_background_color)
		;
	}
};
const action_types_ action_types;

struct visibility_literals_ : x3::symbols<bool>
{
	visibility_literals_()
	{
		add
			(lang::constants::keywords::show, true)
			(lang::constants::keywords::hide, false)
		;
	}

};
const visibility_literals_ visibility_literals;


constexpr auto assignment_operator = '=';
constexpr auto newline_character   = '\n';

}
