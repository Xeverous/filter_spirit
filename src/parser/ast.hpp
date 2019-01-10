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

struct boolean : x3::position_tagged
{
	bool value;
};

struct integer : x3::position_tagged
{
	int value;
};

struct opacity : x3::position_tagged
{
	boost::optional<integer> value;
};

struct identifier : x3::position_tagged
{
	std::string value;
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
	integer r;
	integer g;
	integer b;
	opacity a;
};

struct group_literal : x3::position_tagged
{
	// We do not use 'integer' here as this semantically
	// is not an integer. It's more like integer::value - here we
	// just store counts of each color as implementation detail.
	int r;
	int g;
	int b;
	int w;

	bool has_anything() const { return r != 0 || g != 0 || b != 0 || w != 0; }
};

struct string_literal : x3::position_tagged
{
	std::string value;
};

// ----

struct integer_value_expression : x3::variant<integer, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct rarity_value_expression : x3::variant<rarity_literal, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct shape_value_expression : x3::variant<shape_literal, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct suit_value_expression : x3::variant<suit_literal, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct color_value_expression : x3::variant<color_literal, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct group_value_expression : x3::variant<group_literal, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct string_value_expression : x3::variant<string_literal, identifier>
{
	using base_type::base_type;
	using base_type::operator=;
};

// ----

struct constant_boolean_definition : x3::position_tagged
{
	identifier name;
	boolean value;
};

struct constant_number_definition : x3::position_tagged
{
	identifier name;
	integer_value_expression value;
};

struct constant_level_definition : x3::position_tagged
{
	identifier name;
	integer_value_expression value;
};

struct constant_sound_id_definition : x3::position_tagged
{
	identifier name;
	integer_value_expression value;
};

struct constant_volume_definition : x3::position_tagged
{
	identifier name;
	integer_value_expression value;
};

struct constant_rarity_definition : x3::position_tagged
{
	identifier name;
	rarity_value_expression value;
};

struct constant_shape_definition : x3::position_tagged
{
	identifier name;
	shape_value_expression value;
};

struct constant_suit_definition : x3::position_tagged
{
	identifier name;
	suit_value_expression value;
};

struct constant_color_definition : x3::position_tagged
{
	identifier name;
	color_value_expression value;
};

struct constant_group_definition : x3::position_tagged
{
	identifier name;
	group_value_expression value;
};

struct constant_string_definition : x3::position_tagged
{
	identifier name;
	string_value_expression value;
};

struct constant_definition : x3::variant<
	constant_boolean_definition,
	constant_number_definition,
	constant_level_definition,
	constant_sound_id_definition,
	constant_volume_definition,
	constant_rarity_definition,
	constant_shape_definition,
	constant_suit_definition,
	constant_color_definition,
	constant_group_definition,
	constant_string_definition
>
{
	using base_type::base_type;
	using base_type::operator=;
};

struct code_line : x3::position_tagged
{
	boost::optional<constant_definition> value;
};

using ast_type = std::vector<code_line>;

}
