#pragma once

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
			("True", true)
			("False", false)
		;
	}

};
const booleans_ booleans;

struct rarities_ : x3::symbols<ast::rarity_type>
{
	rarities_()
	{
		add
			("Normal", ast::rarity_type::normal)
			("Magic",  ast::rarity_type::magic)
			("Rare",   ast::rarity_type::rare)
			("Unique", ast::rarity_type::unique)
		;
	}
};
const rarities_ rarities;

struct shapes_ : x3::symbols<ast::shape_type>
{
	shapes_()
	{
		add
			("Circle",   ast::shape_type::circle)
			("Diamond",  ast::shape_type::diamond)
			("Hexagon",  ast::shape_type::hexagon)
			("Square",   ast::shape_type::square)
			("Star",     ast::shape_type::star)
			("Triangle", ast::shape_type::triangle)
		;
	}
};
const shapes_ shapes;

struct suits_ : x3::symbols<ast::suit_type>
{
	suits_()
	{
		add
			("Red",    ast::suit_type::red)
			("Green",  ast::suit_type::green)
			("Blue",   ast::suit_type::blue)
			("White",  ast::suit_type::white)
			("Brown",  ast::suit_type::brown)
			("Yellow", ast::suit_type::yellow)
		;
	}

};
const suits_ suits;


// some constants to aovid code duplication
constexpr auto keyword_boolean  = "Boolean";
constexpr auto keyword_number   = "Number";
constexpr auto keyword_level    = "Level";
constexpr auto keyword_sound_id = "SoundId";
constexpr auto keyword_volume   = "Volume";
constexpr auto keyword_rarity   = "Rarity";
constexpr auto keyword_shape    = "Shape";
constexpr auto keyword_suit     = "Suit";
constexpr auto keyword_color    = "Color";
constexpr auto keyword_group    = "Group";
constexpr auto keyword_string   = "String";
constexpr auto keyword_true     = "True";
constexpr auto keyword_false    = "False";
constexpr auto assignment_operator = '=';
constexpr auto newline_character   = '\n';

}
