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
#include <utility>

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

// ---- whitespace ----

// all whitespace and comments are ignored
// no point in saving them, hence nothing here

// ---- fundamental tokens ----

struct identifier : x3::position_tagged
{
	identifier& operator=(std::string str)
	{
		value = std::move(str);
		return *this;
	}

	const std::string& get_value() const { return value; }

	std::string value;
};

// ---- version requirement ----

struct version_literal : x3::position_tagged
{
	int major = 0;
	int minor = 0;
	int patch = 0;
};

struct version_requirement_statement : x3::position_tagged
{
	version_requirement_statement& operator=(version_literal vl)
	{
		min_required_version = vl;
		return *this;
	}

	const version_literal& get_value() const { return min_required_version; }

	version_literal min_required_version;
};

// ---- config ----

struct config_param : x3::position_tagged
{
	identifier name;
	bool enabled;
	std::vector<config_param> child_params; // yo dawg, I heard you like recursion...
};

struct config : x3::position_tagged
{
	std::vector<config_param> params;
};

// ---- literal types ----

struct integer_literal : x3::position_tagged
{
	integer_literal& operator=(int n)
	{
		value = n;
		return *this;
	}

	int get_value() const { return value; }

	int value;
};

struct string_literal : std::string, x3::position_tagged
{
};

struct boolean_literal : x3::position_tagged
{
	boolean_literal& operator=(bool b)
	{
		value = b;
		return *this;
	}

	bool get_value() const { return value; }

	bool value;
};

struct rarity_literal : x3::position_tagged
{
	rarity_literal& operator=(lang::rarity r)
	{
		value = r;
		return *this;
	}

	lang::rarity get_value() const { return value; }

	lang::rarity value;
};

struct shape_literal : x3::position_tagged
{
	shape_literal& operator=(lang::shape s)
	{
		value = s;
		return *this;
	}

	lang::shape get_value() const { return value; }

	lang::shape value;
};

struct suit_literal : x3::position_tagged
{
	suit_literal& operator=(lang::suit s)
	{
		value = s;
		return *this;
	}

	lang::suit get_value() const { return value; }

	lang::suit value;
};

// ---- expressions ----

struct literal_expression : x3::variant<
		integer_literal,
		string_literal,
		boolean_literal,
		rarity_literal,
		shape_literal,
		suit_literal
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct constructor_call : x3::position_tagged
{
	identifier type_name;
	std::vector<struct value_expression> arguments;
};

struct array_expression : std::vector<struct value_expression>, x3::position_tagged
{
};

struct value_expression : x3::variant<
		literal_expression,
		array_expression,
		constructor_call,
		identifier
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

// core tokens

struct opacity_literal : x3::position_tagged
{
	boost::optional<integer_literal> value;
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

// ----

struct comparison_operator_expression : x3::position_tagged
{
	lang::comparison_type comparison;
};

// ---- conditions ----

struct level_expression : x3::variant<
		integer_literal,
		identifier
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct item_level_condition : x3::position_tagged
{
	lang::comparison_type comparison;
	level_expression level;
};

struct drop_level_condition : x3::position_tagged
{
	lang::comparison_type comparison;
	level_expression level;
};

struct condition_expression : x3::variant<
		item_level_condition,
		drop_level_condition
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

// ---- actions

struct visibility_action : x3::position_tagged
{
	bool show;
};

struct color_expression : x3::variant<
		color_literal,
		identifier
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct border_color_action : x3::position_tagged
{
	color_expression color;
};

struct text_color_action : x3::position_tagged
{
	color_expression color;
};

struct background_color_action : x3::position_tagged
{
	color_expression color;
};

struct action_expression : x3::variant<
		border_color_action,
		text_color_action,
		background_color_action
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

// ---- core structure ----

struct condition_list : x3::position_tagged
{
	std::vector<condition_expression> condition_expressions;
};

struct action_list : x3::position_tagged
{
	std::vector<action_expression> action_expressions;
};

struct constant_definition : x3::position_tagged
{
	identifier name;
	value_expression value;
};

struct constant_definition_list : x3::position_tagged
{
	std::vector<constant_definition> constant_definitions;
};

struct rule_block_list : x3::position_tagged
{
	std::vector<struct rule_block> blocks;
};

struct rule_block : x3::position_tagged
{
	condition_list conditions;
	action_list actions;
	rule_block_list nested_blocks;
};

struct filter_specification : x3::position_tagged
{
	version_requirement_statement version_data;
	config config;
	constant_definition_list constants_list;
	action_list actions;
	rule_block_list blocks;
};

using ast_type = filter_specification;

}
