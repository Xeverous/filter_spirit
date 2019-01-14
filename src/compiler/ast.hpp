#pragma once

#include "parser/ast.hpp"
#include <boost/optional.hpp>
#include <string>
#include <variant>

namespace fs::compiler::ast
{

struct boolean
{
	bool value;
};

struct number
{
	int value;
};

struct level
{
	int value;
};

struct sound_id
{
	int value;
};

struct volume
{
	int value;
};

struct color
{
	int r;
	int g;
	int b;
	boost::optional<int> a;
};

using rarity_type = fs::parser::ast::rarity_type;
struct rarity
{
	rarity_type value;
};

using shape_type = fs::parser::ast::shape_type;
struct shape
{
	shape_type value;
};

using suit_type = fs::parser::ast::suit_type;
struct suit
{
	suit_type value;
};

struct group
{
	int r;
	int g;
	int b;
	int w;
};

struct string
{
	std::string value;
};

using object_type = std::variant<
	boolean,
	number,
	level,
	sound_id,
	volume,
	color,
	rarity,
	shape,
	suit,
	group,
	string
>;

}
