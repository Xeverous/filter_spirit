/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

/*
 * Warning: std::optional and std::tuple are not yet supported,
 * use Boost counterparts instead.
 *
 * Do not use std::variant, inherit from x3::variant instead.
 *
 * https://github.com/boostorg/spirit/issues/270
 */

namespace fs::ast
{

namespace x3 = boost::spirit::x3;

// core tokens

struct boolean_literal : x3::position_tagged
{
	bool value;
};

struct integer_literal : x3::position_tagged
{
	int value;
};

struct opacity_literal : x3::position_tagged
{
	boost::optional<integer_literal> value;
};

enum class rarity_type { normal, magic, rare, unique };
struct rarity_literal : x3::position_tagged
{
	rarity_type value;
};

enum class shape_type { circle, diamond, hexagon, square, star, triangle };
struct shape_literal : x3::position_tagged
{
	shape_type value;
};

enum class suit_type { red, green, blue, white, brown, yellow };
struct suit_literal : x3::position_tagged
{
	suit_type value;
};

struct color_literal : x3::position_tagged
{
	integer_literal r;
	integer_literal g;
	integer_literal b;
	opacity_literal a;
};

struct group_literal : x3::position_tagged
{
	// We do not use 'integer_literal' here as this semantically
	// is not an integer_literal. It's more like integer_literal::value - here we
	// just store counts of each color as implementation detail.
	int r;
	int g;
	int b;
	int w;

	bool has_anything() const { return r != 0 || g != 0 || b != 0 || w != 0; }
};

struct identifier : x3::position_tagged
{
	std::string value;
};

struct string_literal : x3::position_tagged
{
	std::string value;
};

// ----

enum class object_type_type
{
	boolean,
	number,
	level,
	sound_id,
	volume,
	rarity,
	shape,
	suit,
	color,
	group,
	identifier,
	string
};

struct object_type_expression : x3::position_tagged
{
	object_type_type value;
};

struct value_expression : x3::variant<
		boolean_literal,
		rarity_literal,
		shape_literal,
		suit_literal,
		color_literal,
		string_literal,
		integer_literal,
		identifier
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

// ----

struct constant_definition : x3::position_tagged
{
	object_type_expression object_type;
	identifier name;
	value_expression value;
};


struct code_line : x3::position_tagged
{
	boost::optional<constant_definition> value;
};

using ast_type = std::vector<code_line>;

}
