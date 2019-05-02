/**
 * @file grammar type definitions
 *
 * @details This file is intended for core grammar definitions.
 * Use BOOST_SPIRIT_DEFINE here.
 */
#pragma once
#include "fs/parser/detail/symbols.hpp"
#include "fs/parser/detail/grammar.hpp"

namespace fs::parser::detail
{

// Spirit coding style:
// - rule ID        : foo_class
// - rule type      : foo_type
// - rule definition: foo_def
// - rule object    : foo

// ---- whitespace ----

const comment_type comment = "comment";
const auto comment_def = x3::lit('#') >> *(x3::char_ - x3::eol) >> (x3::eol | x3::eoi);
BOOST_SPIRIT_DEFINE(comment)

const whitespace_type whitespace = "whitespace";
const auto whitespace_def = x3::space | comment;
BOOST_SPIRIT_DEFINE(whitespace)

// ---- fundamental tokens ----

const identifier_impl_type identifier_impl = "identifier";
const auto identifier_impl_def = x3::lexeme[(x3::alpha | x3::char_('_')) > *(x3::alnum | x3::char_('_'))];
BOOST_SPIRIT_DEFINE(identifier_impl)

const identifier_type identifier = identifier_impl.name;
const auto identifier_def = identifier_impl;
BOOST_SPIRIT_DEFINE(identifier)

// ---- version requirement ----

const version_literal_type version_literal = "version literal";
const auto version_literal_def = x3::int_ >> '.' >> x3::int_ >> '.' >> x3::int_;
BOOST_SPIRIT_DEFINE(version_literal)

const version_requirement_statement_type version_requirement_statement = "version requirement statement";
const auto version_requirement_statement_def = x3::lit(lang::keywords::version) > ':' > version_literal;
BOOST_SPIRIT_DEFINE(version_requirement_statement)

// ---- config ----

const config_param_type config_param = "config param";
const auto config_param_def = identifier > ':' > symbols::yes_no > (('{' > *config_param > '}') | x3::attr(std::vector<ast::config_param>()));
BOOST_SPIRIT_DEFINE(config_param)

const config_type config = "config";
const auto config_def = x3::lit(lang::keywords::config) > ':' > '{' > *config_param > '}';
BOOST_SPIRIT_DEFINE(config)

// ---- literal types ----

const floating_point_literal_type floating_point_literal = "number (fractional)";
const auto floating_point_literal_def = x3::real_parser<double, x3::strict_real_policies<double>>{};
BOOST_SPIRIT_DEFINE(floating_point_literal)

const integer_literal_type integer_literal = "number (integer)";
const auto integer_literal_def = x3::int_;
BOOST_SPIRIT_DEFINE(integer_literal)

const string_literal_type string_literal = "string";
const auto string_literal_def = x3::lexeme['"' > *(x3::char_ - '"') > '"'];
BOOST_SPIRIT_DEFINE(string_literal)

const boolean_literal_type boolean_literal = "boolean literal";
const auto boolean_literal_def = symbols::booleans;
BOOST_SPIRIT_DEFINE(boolean_literal)

const rarity_literal_type rarity_literal = "rarity literal";
const auto rarity_literal_def = symbols::rarities;
BOOST_SPIRIT_DEFINE(rarity_literal)

const shape_literal_type shape_literal = "shape literal";
const auto shape_literal_def = symbols::shapes;
BOOST_SPIRIT_DEFINE(shape_literal)

const suit_literal_type suit_literal = "suit literal";
const auto suit_literal_def = symbols::suits;
BOOST_SPIRIT_DEFINE(suit_literal)

// ---- expressions ----

const literal_expression_type literal_expression = "literal";
const auto literal_expression_def =
// order here is crucial in context-sensitive grammars
// this grammar is context-free but the order will affect performance
// we can clearly assume that integers and strings will be the most popular
// note: order should match types in literal_expression_type::attribute_type
	  floating_point_literal
	| integer_literal
	| string_literal
	| boolean_literal
	| rarity_literal
	| shape_literal
	| suit_literal;
BOOST_SPIRIT_DEFINE(literal_expression)

// moved here due to circular dependency
const value_expression_type value_expression = "expression";

const value_expression_list_type value_expression_list = "expression list";
const auto value_expression_list_def = (value_expression % ',') | x3::attr(ast::value_expression_list());
BOOST_SPIRIT_DEFINE(value_expression_list)

const function_call_type function_call = "function call";
const auto function_call_def = identifier >> '(' >> value_expression_list >> ')';
BOOST_SPIRIT_DEFINE(function_call)

const price_range_query_type price_range_query = "price range query";
const auto price_range_query_def = '$' >> identifier >> '(' >> value_expression_list >> ')';
BOOST_SPIRIT_DEFINE(price_range_query)

const array_expression_type array_expression = "array expression";
const auto array_expression_def = '[' > value_expression_list > ']';
BOOST_SPIRIT_DEFINE(array_expression)

const auto value_expression_def =
	  literal_expression
	| array_expression
	| function_call
	| identifier
	| price_range_query;
BOOST_SPIRIT_DEFINE(value_expression)

// ---- definitions ----

const constant_definition_type constant_definition = "constant definiton";
const auto constant_definition_def = lang::keywords::const_ > identifier > '=' > value_expression;
BOOST_SPIRIT_DEFINE(constant_definition)

// ---- rules ----

const comparison_operator_expression_type comparison_operator_expression = "comparison operator";
const auto comparison_operator_expression_def = symbols::comparison_operators | x3::attr(lang::comparison_type::equal);
BOOST_SPIRIT_DEFINE(comparison_operator_expression)

const comparison_condition_type comparison_condition = "comparison condition";
const auto comparison_condition_def = symbols::comparison_condition_properties > comparison_operator_expression > value_expression;
BOOST_SPIRIT_DEFINE(comparison_condition)

const string_condition_type string_condition = "string condition";
const auto string_condition_def = symbols::string_condition_properties > value_expression;
BOOST_SPIRIT_DEFINE(string_condition)

const boolean_condition_type boolean_condition = "boolean condition";
const auto boolean_condition_def = symbols::boolean_condition_properties > value_expression;
BOOST_SPIRIT_DEFINE(boolean_condition)

const socket_group_condition_type socket_group_condition = "socket group condition";
const auto socket_group_condition_def = lang::keywords::socket_group > value_expression;
BOOST_SPIRIT_DEFINE(socket_group_condition)

const condition_type condition = "condition";
const auto condition_def =
	  comparison_condition
	| string_condition
	| boolean_condition
	| socket_group_condition;
BOOST_SPIRIT_DEFINE(condition)

const action_type action= "action";
const auto action_def = symbols::action_types > value_expression;
BOOST_SPIRIT_DEFINE(action)

// ---- filter structure ----

const visibility_statement_type visibility_statement = "visibility statement";
const auto visibility_statement_def = symbols::visibility_literals;
BOOST_SPIRIT_DEFINE(visibility_statement)

// moved here due to circular dependency
const rule_block_type rule_block = "rule block";

const statement_type statement = "statement";
const auto statement_def = action | visibility_statement | rule_block;
BOOST_SPIRIT_DEFINE(statement)

const auto rule_block_def = ((condition % "&&") | x3::attr(std::vector<ast::condition>())) >> x3::lit('{') > *statement > x3::lit('}');
BOOST_SPIRIT_DEFINE(rule_block)

const filter_structure_type filter_structure = "filter structure";
const auto filter_structure_def = version_requirement_statement > config > *constant_definition > *statement;
BOOST_SPIRIT_DEFINE(filter_structure)

const grammar_type grammar = "code";
const auto grammar_def = filter_structure > x3::eoi;
BOOST_SPIRIT_DEFINE(grammar)

}
