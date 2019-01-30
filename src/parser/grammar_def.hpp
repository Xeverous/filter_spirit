/**
 * @file grammar type definitions
 *
 * @details This file is intended for core grammar definitions.
 * Use BOOST_SPIRIT_DEFINE here.
 */
#pragma once
#include "parser/grammar.hpp"
#include "parser/constants.hpp"
#include "parser/count.hpp"
#include <type_traits>

namespace
{

// Validates that a group makes sense. Any color can be specified to appear 0 times,
// but not all at once as this would form an empty group.
const auto validate_group = [](auto& context)
{
	using attribute_type = std::remove_reference_t<decltype(_attr(context))>;
	static_assert(
		std::is_same_v<fs::parser::ast::group_literal, attribute_type>,
		"Only for validating group type objects");

	if (!_attr(context).has_anything())
		_pass(context) = false;
};

const auto set_compare_equal = [](auto& context)
{
	using attribute_type = std::remove_reference_t<decltype(_attr(context))>;
	static_assert(
		std::is_same_v<fs::parser::ast::comparison_operator_expression, attribute_type>,
		"Only setting up comparison operator");

	_val(context) = fs::lang::comparison_type::equal;
};

}

namespace fs::parser
{

// Spirit coding style:
// - rule ID        : foo_class
// - rule type      : foo_type
// - rule definition: foo_def
// - rule object    : foo

const whitespace_type whitespace = "whitespace";
const auto whitespace_def = x3::space - newline_character;
BOOST_SPIRIT_DEFINE(whitespace)

const comment_type comment = "comment";
const auto comment_def = x3::lexeme['#' > *(x3::char_ - newline_character)];
BOOST_SPIRIT_DEFINE(comment)

const boolean_type boolean_literal = "boolean ('True' OR 'False')";
const auto boolean_literal_def = booleans;
BOOST_SPIRIT_DEFINE(boolean_literal)

const integer_type integer_literal = "integer";
const auto integer_literal_def = x3::int_;
BOOST_SPIRIT_DEFINE(integer_literal)

const opacity_type opacity_literal = "opacity";
const auto opacity_literal_def = -integer_literal;
BOOST_SPIRIT_DEFINE(opacity_literal)

const rarity_literal_type rarity_literal = "rarity literal";
const auto rarity_literal_def = rarities;
BOOST_SPIRIT_DEFINE(rarity_literal)

const shape_literal_type shape_literal = "shape literal";
const auto shape_literal_def = shapes;
BOOST_SPIRIT_DEFINE(shape_literal)

const suit_literal_type suit_literal = "suit literal";
const auto suit_literal_def = suits;
BOOST_SPIRIT_DEFINE(suit_literal)

const color_literal_type color_literal = "color (3 or 4 integers)";
const auto color_literal_def = integer_literal >> integer_literal >> integer_literal >> opacity_literal;
BOOST_SPIRIT_DEFINE(color_literal)

const group_literal_impl_type group_literal_impl = "group (R/G/B/W letters in this order)";
const auto group_literal_impl_def = x3ext::count['R'] >> x3ext::count['G'] >> x3ext::count['B'] >> x3ext::count['W'];
BOOST_SPIRIT_DEFINE(group_literal_impl)

const group_literal_type group_literal =  group_literal_impl.name;
const auto group_literal_def = group_literal_impl[validate_group];
BOOST_SPIRIT_DEFINE(group_literal)

const identifier_impl_type identifier_impl = "identifier";
const auto identifier_impl_def = x3::lexeme[(x3::alpha | x3::char_('_')) > *(x3::alnum | x3::char_('_'))];
BOOST_SPIRIT_DEFINE(identifier_impl)

const identifier_type identifier = identifier_impl.name;
const auto identifier_def = identifier_impl;
BOOST_SPIRIT_DEFINE(identifier)

const string_literal_type string_literal = "string";
const auto string_literal_def = x3::lexeme['"' > +(x3::char_ - '"') > '"'];
BOOST_SPIRIT_DEFINE(string_literal)

// ----

const object_type_expression_type object_type_expression = "type name";
const auto object_type_expression_def = object_types;
BOOST_SPIRIT_DEFINE(object_type_expression)

const array_type_expression_type array_type_expression = "array type name";
const auto array_type_expression_def = x3::lit(lang::constants::keywords::array) > x3::lit('<') > object_types > x3::lit('>');
BOOST_SPIRIT_DEFINE(array_type_expression)

const type_expression_type type_expression = "type name";
const auto type_expression_def = object_type_expression | array_type_expression;
BOOST_SPIRIT_DEFINE(type_expression)

// ----

const literal_expression_type literal_expression = "literal";
const auto literal_expression_def =
// order has a big matter here
// more complex grammars should be first, otherwise the parser
// could mistakenly treat insufficient amount of tokens
// eg parsing 101 102 103 as integer + 2 unwanted tokens instead of color
	  boolean_literal
	| rarity_literal
	| shape_literal
	| suit_literal
	| color_literal
	| string_literal
	| integer_literal;
BOOST_SPIRIT_DEFINE(literal_expression)

// circular reference, need to change order
const array_expression_type array_expression = "array expression";

const value_expression_type value_expression = "expression";
const auto value_expression_def =
	  literal_expression
	| identifier
	| array_expression;
BOOST_SPIRIT_DEFINE(value_expression)

const auto array_expression_def = x3::lit('[') > (value_expression % x3::lit(',')) > x3::lit(']');
BOOST_SPIRIT_DEFINE(array_expression)

// ----

const comparison_operator_expression_type comparison_operator_expression = "comparison operator";
const auto comparison_operator_expression_def = comparison_operators | x3::eps[set_compare_equal];
BOOST_SPIRIT_DEFINE(comparison_operator_expression)

// ----

const constant_definition_type constant_definition = "constant definiton";
const auto constant_definition_def = type_expression > identifier > x3::lit(assignment_operator) > value_expression;
BOOST_SPIRIT_DEFINE(constant_definition)

// ----

const level_expression_type level_expression = "level expression";
const auto level_expression_def = integer_literal | identifier;
BOOST_SPIRIT_DEFINE(level_expression)

const item_level_condition_type item_level_condition = "item level condition";
const auto item_level_condition_def = x3::lit(lang::constants::keywords::item_level) > comparison_operator_expression > level_expression;
BOOST_SPIRIT_DEFINE(item_level_condition)

const drop_level_condition_type drop_level_condition = "drop level condition";
const auto drop_level_condition_def = x3::lit(lang::constants::keywords::drop_level) > comparison_operator_expression > level_expression;
BOOST_SPIRIT_DEFINE(drop_level_condition)

const condition_expression_type condition_expression = "condition expression";
const auto condition_expression_def =
	  item_level_condition
	| drop_level_condition;
BOOST_SPIRIT_DEFINE(condition_expression)

// ----

const visibility_action_type visibility_action = "visibility action";
const auto visibility_action_def = visibility_literals;
BOOST_SPIRIT_DEFINE(visibility_action)

const color_expression_type color_expression = "color expression";
const auto color_expression_def = color_literal | identifier;
BOOST_SPIRIT_DEFINE(color_expression)

const border_color_action_type border_color_action = "border color action";
const auto border_color_action_def = lang::constants::keywords::set_border_color > color_expression;
BOOST_SPIRIT_DEFINE(border_color_action)

const text_color_action_type text_color_action = "text color action";
const auto text_color_action_def = lang::constants::keywords::set_text_color > color_expression;
BOOST_SPIRIT_DEFINE(text_color_action)

const background_color_action_type background_color_action = "background color action";
const auto background_color_action_def = lang::constants::keywords::set_background_color > color_expression;
BOOST_SPIRIT_DEFINE(background_color_action)

const action_expression_type action_expression = "action expression";
const auto action_expression_def =
	  border_color_action
	| text_color_action
	| background_color_action;
BOOST_SPIRIT_DEFINE(action_expression)

// ----

const condition_list_type condition_list = "condition list";
const auto condition_list_def = condition_expression % x3::lit(',');
BOOST_SPIRIT_DEFINE(condition_list)

const action_list_type action_list = "action list";
const auto action_list_def = *action_expression;
BOOST_SPIRIT_DEFINE(action_list)

// circular reference
const condition_block_list_type condition_block_list = "condition block list";

const condition_block_type condition_block = "condition block";
const auto condition_block_def = condition_list > x3::lit('{') > action_list > condition_block_list > x3::lit('}');
BOOST_SPIRIT_DEFINE(condition_block)

const auto condition_block_list_def = *condition_block;
BOOST_SPIRIT_DEFINE(condition_block_list)

// ----

const constant_definition_line_type constant_definition_line = "constant definition line";
const auto constant_definition_line_def = (constant_definition | x3::eps) >> -comment >> x3::eol;
BOOST_SPIRIT_DEFINE(constant_definition_line)

const grammar_type grammar = "code";
const auto grammar_def = *constant_definition_line > x3::eoi;
BOOST_SPIRIT_DEFINE(grammar)

}

