#pragma once
#include "parser/ast.hpp"
#include "lang/types.hpp"
#include "lang/constants.hpp"
#include <boost/spirit/home/x3.hpp>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;

// symbols that denote filter's language constants
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

struct object_types_ : x3::symbols<lang::object_type>
{
	object_types_()
	{
		add
			(lang::constants::keywords::boolean,  lang::object_type::boolean)
			(lang::constants::keywords::number,   lang::object_type::number)
			(lang::constants::keywords::level,    lang::object_type::level)
			(lang::constants::keywords::sound_id, lang::object_type::sound_id)
			(lang::constants::keywords::volume,   lang::object_type::volume)
			(lang::constants::keywords::rarity,   lang::object_type::rarity)
			(lang::constants::keywords::shape,    lang::object_type::shape)
			(lang::constants::keywords::suit,     lang::object_type::suit)
			(lang::constants::keywords::color,    lang::object_type::color)
			(lang::constants::keywords::group,    lang::object_type::group)
			(lang::constants::keywords::string,   lang::object_type::string)
		;
	}
};
const object_types_ object_types;


constexpr auto assignment_operator = '=';
constexpr auto newline_character   = '\n';

}
