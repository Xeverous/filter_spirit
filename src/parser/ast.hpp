/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once
#include "lang/types.hpp"
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

namespace fs::parser::ast
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

struct rarity_literal : x3::position_tagged
{
	lang::rarity value;
};

struct shape_literal : x3::position_tagged
{
	lang::shape value;
};

struct suit_literal : x3::position_tagged
{
	lang::suit value;
};

struct color_literal : x3::position_tagged
{
	integer_literal r;
	integer_literal g;
	integer_literal b;
	opacity_literal a;

	lang::color to_lang_color() const
	{
		fs::lang::color result;
		result.r = r.value;
		result.g = g.value;
		result.b = b.value;

		if (a.value)
			result.a = (*a.value).value;
		else
			result.a = std::nullopt;

		return result;
	}
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

struct object_type_expression : x3::position_tagged
{
	lang::single_object_type value;
};

struct array_type_expression : x3::position_tagged
{
	lang::single_object_type value;
};

struct type_expression : x3::variant<
		object_type_expression,
		array_type_expression
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

// ----

struct literal_expression : x3::variant<
		boolean_literal,
		rarity_literal,
		shape_literal,
		suit_literal,
		color_literal,
		string_literal,
		integer_literal
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct value_expression : x3::variant<
		literal_expression,
		identifier,
		x3::forward_ast<struct array_expression>
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct array_expression : x3::position_tagged
{
	std::vector<value_expression> values;
};

// ----

struct constant_definition : x3::position_tagged
{
	type_expression type;
	identifier name;
	value_expression value;
};

struct code_line : x3::position_tagged
{
	boost::optional<constant_definition> value;
};

using ast_type = std::vector<code_line>;

}
