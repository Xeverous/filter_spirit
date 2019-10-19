/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once

#include <fs/lang/types.hpp>

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <utility>
#include <string>
#include <type_traits>

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

struct floating_point_literal : x3::position_tagged
{
	floating_point_literal& operator=(double n)
	{
		value = n;
		return *this;
	}

	double get_value() const { return value; }

	double value;
};

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
		floating_point_literal,
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

struct value_expression_list : std::vector<struct value_expression>, x3::position_tagged
{
};

struct function_call : x3::position_tagged
{
	identifier name;
	value_expression_list arguments;
};

struct price_range_query : x3::position_tagged
{
	identifier name;
	value_expression_list arguments;
};

struct array_expression : x3::position_tagged
{
	array_expression& operator=(value_expression_list list)
	{
		elements = std::move(list);
		return *this;
	}

	const value_expression_list& get_value() const { return elements; }

	value_expression_list elements;
};

struct primary_expression : x3::variant<
		literal_expression,
		array_expression,
		function_call,
		identifier,
		price_range_query
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

/*
 * https://github.com/boostorg/spirit/issues/465
 * There are 2 bugs in GCC that may cause problems:
 *
 * GCC 6.4 - 9.0-: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89381
 * Using declarations for base_type's operator= and base_type's ctor in types
 * inheritted from base_type which was a class template did not actually
 * created copy/move ctor/operators - ending in very unexpected compiler errors.
 * that an object can not be copied/moved or copy/move assigned.
 *
 * Workaround: place these usings after each problematic type T definition,
 * they will add missing copy and move ctors.
 * using workaround1 = decltype(T{*static_cast<const T*>(nullptr)});
 * using workaround2 = decltype(T{static_cast<T&&>(*static_cast<T*>(nullptr))});
 *
 * GCC 9.0+: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92145
 * The above bug is fixed but the newly added -Wdeprecated-copy has
 * a false positive on the same type definition. Workaround above does not help.
 * Simply use -Wno-deprecated-copy.
 */

struct postfix_expression;

struct value_expression : x3::position_tagged
{
	primary_expression primary_expr;
	std::vector<postfix_expression> postfix_exprs;
};

struct subscript : x3::position_tagged
{
	subscript& operator=(value_expression ve)
	{
		expr = std::move(ve);
		return *this;
	}

	const value_expression& get_value() const { return expr; }

	value_expression expr;
};

struct postfix_expression : x3::position_tagged
{
	postfix_expression& operator=(subscript s)
	{
		expr = std::move(s);
		return *this;
	}

	const subscript& get_value() const { return expr; }

	subscript expr;
};

// ---- definitions ----

struct constant_definition : x3::position_tagged
{
	identifier name;
	value_expression value;
};

// ---- rules ----

struct comparison_operator_expression : x3::position_tagged
{
	comparison_operator_expression& operator=(lang::comparison_type ct)
	{
		value = ct;
		return *this;
	}

	lang::comparison_type get_value() const { return value; }

	lang::comparison_type value;
};

struct comparison_condition : x3::position_tagged
{
	lang::comparison_condition_property property;
	comparison_operator_expression comparison_type;
	value_expression value;
};

struct exact_matching_policy : x3::position_tagged
{
	exact_matching_policy& operator=(bool v)
	{
		required = v;
		return *this;
	}

	bool get_value() const { return required; }

	bool required;
};

struct string_condition : x3::position_tagged
{
	lang::string_condition_property property;
	exact_matching_policy exact_match;
	value_expression value;
};

struct boolean_condition : x3::position_tagged
{
	lang::boolean_condition_property property;
	value_expression value;
};

struct socket_group_condition : x3::position_tagged
{
	socket_group_condition& operator=(value_expression ve)
	{
		value = std::move(ve);
		return *this;
	}

	const value_expression& get_value() const { return value; }

	value_expression value;
};

struct condition : x3::variant<
		comparison_condition,
		string_condition,
		boolean_condition,
		socket_group_condition
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct unary_action : x3::position_tagged
{
	lang::unary_action_type action_type;
	value_expression value;
};

struct action : x3::position_tagged
{
	action& operator=(unary_action ua)
	{
		action = std::move(ua);
		return *this;
	}

	const unary_action& get_value() const { return action; }

	unary_action action;
};

// ---- filter structure ----

struct visibility_statement : x3::position_tagged
{
	visibility_statement operator=(bool value)
	{
		show = value;
		return *this;
	}

	bool get_value() const { return show; }

	bool show;
};

// rule_block defined earlier than statement due to circular dependency
// note: we could use x3::forward_ast but it would have a worse memory layout
struct rule_block : x3::position_tagged
{
	std::vector<condition> conditions;
	std::vector<struct statement> statements;
};

struct statement : x3::variant<
		action,
		visibility_statement,
		rule_block
	>, x3::position_tagged
{
	using base_type::base_type;
	using base_type::operator=;
};

struct filter_structure : x3::position_tagged
{
	version_requirement_statement version_data;
	config config;
	std::vector<constant_definition> constant_definitions;
	std::vector<statement> statements;
};

using ast_type = filter_structure;

}

namespace fs::parser
{
	template <typename T>
	boost::spirit::x3::position_tagged get_position_info(const T& ast)
	{
		static_assert(std::is_base_of_v<boost::spirit::x3::position_tagged, T>, "T must be derived from position_tagged");
		// intentional object slicing
		return static_cast<boost::spirit::x3::position_tagged>(ast);
	}
}
