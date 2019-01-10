/**
 * @file grammar type definitions
 *
 * @details This file is intended for core grammar definitions.
 * Use BOOST_SPIRIT_DEFINE here.
 */
#pragma once
#include "grammar.hpp"
#include "constants.hpp"
#include "count.hpp"
#include <type_traits>

namespace
{

// Validates that a group makes sense. Any color can be specified to appear 0 times,
// but not all at once as this would form an empty group.
const auto validate_group = [](auto& context)
{
	using attribute_type = std::remove_reference_t<decltype(_attr(context))>;
	static_assert(
		std::is_same_v<fs::ast::group_literal, attribute_type>,
		"Only for validating group type objects");

	if (!_attr(context).has_anything())
		_pass(context) = false;
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

const boolean_type boolean = "boolean ('True' OR 'False')";
const auto boolean_def = booleans;
BOOST_SPIRIT_DEFINE(boolean)

const integer_type integer = "integer";
const auto integer_def = x3::int_;
BOOST_SPIRIT_DEFINE(integer)

const opacity_type opacity = "opacity";
const auto opacity_def = -integer;
BOOST_SPIRIT_DEFINE(opacity)

const identifier_impl_type identifier_impl = "identifier implementation";
const auto identifier_impl_def = x3::lexeme[(x3::alpha | '_') > *(x3::alnum | '_')];
BOOST_SPIRIT_DEFINE(identifier_impl)

const identifier_type identifier = "identifier";
const auto identifier_def = identifier_impl;
BOOST_SPIRIT_DEFINE(identifier)

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
const auto color_literal_def = integer >> integer >> integer >> opacity;
BOOST_SPIRIT_DEFINE(color_literal)

const group_literal_impl_type group_literal_impl = "group (R/G/B/W letters in this order)";
const auto group_literal_impl_def = x3ext::count['R'] >> x3ext::count['G'] >> x3ext::count['B'] >> x3ext::count['W'];
BOOST_SPIRIT_DEFINE(group_literal_impl)

const group_literal_type group_literal =  group_literal_impl.name;
const auto group_literal_def = group_literal_impl[validate_group];
BOOST_SPIRIT_DEFINE(group_literal)

const string_literal_type string_literal = "string";
const auto string_literal_def = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
BOOST_SPIRIT_DEFINE(string_literal)

// ----

const integer_value_expression_type integer_value_expression = "integer expression";
const auto integer_value_expression_def = integer | identifier;
BOOST_SPIRIT_DEFINE(integer_value_expression)

const rarity_value_expression_type rarity_value_expression = "rarity expression";
const auto rarity_value_expression_def = rarity_literal | identifier;
BOOST_SPIRIT_DEFINE(rarity_value_expression)

const shape_value_expression_type shape_value_expression = "shape expression";
const auto shape_value_expression_def = shape_literal | identifier;
BOOST_SPIRIT_DEFINE(shape_value_expression)

const suit_value_expression_type suit_value_expression = "suit expression";
const auto suit_value_expression_def = suit_literal | identifier;
BOOST_SPIRIT_DEFINE(suit_value_expression)

const color_value_expression_type color_value_expression = "color expression";
const auto color_value_expression_def = color_literal | identifier;
BOOST_SPIRIT_DEFINE(color_value_expression)

const group_value_expression_type group_value_expression = "group expression";
const auto group_value_expression_def = group_literal | identifier;
BOOST_SPIRIT_DEFINE(group_value_expression)

const string_value_expression_type string_value_expression = "string expression";
const auto string_value_expression_def = string_literal | identifier;
BOOST_SPIRIT_DEFINE(string_value_expression)

// ----

const constant_boolean_definition_type constant_boolean_definition = "Boolean definition";
const auto constant_boolean_definition_def = x3::lit(keyword_boolean) > identifier > x3::lit(assignment_operator) > boolean;
BOOST_SPIRIT_DEFINE(constant_boolean_definition)

const constant_number_definition_type constant_number_definition = "Number definition";
const auto constant_number_definition_def = x3::lit(keyword_number) > identifier > x3::lit(assignment_operator) > integer_value_expression;
BOOST_SPIRIT_DEFINE(constant_number_definition)

const constant_level_definition_type constant_level_definition = "Level definition";
const auto constant_level_definition_def = x3::lit(keyword_level) > identifier > x3::lit(assignment_operator) > integer_value_expression;
BOOST_SPIRIT_DEFINE(constant_level_definition)

const constant_sound_id_definition_type constant_sound_id_definition = "SoundId definition";
const auto constant_sound_id_definition_def = x3::lit(keyword_sound_id) > identifier > x3::lit(assignment_operator) > integer_value_expression;
BOOST_SPIRIT_DEFINE(constant_sound_id_definition)

const constant_volume_definition_type constant_volume_definition = "Volume definition";
const auto constant_volume_definition_def = x3::lit(keyword_volume) > identifier > x3::lit(assignment_operator) > integer_value_expression;
BOOST_SPIRIT_DEFINE(constant_volume_definition)

const constant_rarity_definition_type constant_rarity_definition = "Rarity definition";
const auto constant_rarity_definition_def = x3::lit(keyword_rarity) > identifier > x3::lit(assignment_operator) > rarity_value_expression;
BOOST_SPIRIT_DEFINE(constant_rarity_definition)

const constant_shape_definition_type constant_shape_definition = "Shape definition";
const auto constant_shape_definition_def = x3::lit(keyword_shape) > identifier > x3::lit(assignment_operator) > shape_value_expression;
BOOST_SPIRIT_DEFINE(constant_shape_definition)

const constant_suit_definition_type constant_suit_definition = "Suit definition";
const auto constant_suit_definition_def = x3::lit(keyword_suit) > identifier > x3::lit(assignment_operator) > suit_value_expression;
BOOST_SPIRIT_DEFINE(constant_suit_definition)

const constant_color_definition_type constant_color_definition = "Color definition";
const auto constant_color_definition_def = x3::lit(keyword_color) > identifier > x3::lit(assignment_operator) > color_value_expression;
BOOST_SPIRIT_DEFINE(constant_color_definition)

const constant_group_definition_type constant_group_definition = "Group definition";
const auto constant_group_definition_def = x3::lit(keyword_group) > identifier > x3::lit(assignment_operator) > group_value_expression;
BOOST_SPIRIT_DEFINE(constant_group_definition)

const constant_string_definition_type constant_string_definition = "String definiton";
const auto constant_string_definition_def = x3::lit(keyword_string) > identifier > x3::lit(assignment_operator) > string_value_expression;
BOOST_SPIRIT_DEFINE(constant_string_definition)

const constant_definition_type constant_definition = "constant definition";
const auto constant_definition_def =
	  constant_boolean_definition
	| constant_number_definition
	| constant_level_definition
	| constant_sound_id_definition
	| constant_volume_definition
	| constant_rarity_definition
	| constant_shape_definition
	| constant_suit_definition
	| constant_color_definition
	| constant_group_definition
	| constant_string_definition;
BOOST_SPIRIT_DEFINE(constant_definition)

const code_line_type code_line = "line";
const auto code_line_def = (constant_definition | x3::eps) >> -comment >> x3::eol;
BOOST_SPIRIT_DEFINE(code_line)

const grammar_type grammar = "code";
const auto grammar_def = *code_line > x3::eoi;
BOOST_SPIRIT_DEFINE(grammar)

}

