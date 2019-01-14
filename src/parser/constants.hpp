#pragma once
#include "parser/ast.hpp"
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

namespace keywords
{
	constexpr auto boolean  = "Boolean";
	constexpr auto number   = "Number";
	constexpr auto level    = "Level";
	constexpr auto sound_id = "SoundId";
	constexpr auto volume   = "Volume";
	constexpr auto rarity   = "Rarity";
	constexpr auto shape    = "Shape";
	constexpr auto suit     = "Suit";
	constexpr auto color    = "Color";
	constexpr auto group    = "Group";
	constexpr auto string   = "String";
}

struct object_types_ : x3::symbols<ast::object_type_type>
{
	object_types_()
	{
		add
			(keywords::boolean,  ast::object_type_type::boolean)
			(keywords::number,   ast::object_type_type::number)
			(keywords::level,    ast::object_type_type::level)
			(keywords::sound_id, ast::object_type_type::sound_id)
			(keywords::volume,   ast::object_type_type::volume)
			(keywords::rarity,   ast::object_type_type::rarity)
			(keywords::shape,    ast::object_type_type::shape)
			(keywords::suit,     ast::object_type_type::suit)
			(keywords::color,    ast::object_type_type::color)
			(keywords::group,    ast::object_type_type::group)
			(keywords::string,   ast::object_type_type::string)
		;
	}
};
const object_types_ object_types;


constexpr auto assignment_operator = '=';
constexpr auto newline_character   = '\n';

}
