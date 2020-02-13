/**
 * @file grammar type definitions
 *
 * @details This file is intended for core grammar definitions.
 * Use BOOST_SPIRIT_DEFINE here.
 */
#pragma once
#include <fs/parser/detail/symbols.hpp>
#include <fs/parser/detail/grammar.hpp>

namespace fs::parser::detail
{

// Spirit coding style:
// - rule ID        : foo_class
// - rule type      : foo_type
// - rule definition: foo_def
// - rule object    : foo

namespace common
{
	// ---- whitespace ----

	const comment_type comment = "comment";
	const auto comment_def = x3::lit('#') > *(x3::char_ - x3::eol) > (x3::eol | x3::eoi);
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

	const not_alnum_or_underscore_type not_alnum_or_underscore = "not alphanumeric character or underscore";
	const auto not_alnum_or_underscore_def = !&(x3::alnum | '_');
	BOOST_SPIRIT_DEFINE(not_alnum_or_underscore)

	// ---- literal types ----

	const integer_literal_type integer_literal = "number (integer)";
	const auto integer_literal_def = x3::int_;
	BOOST_SPIRIT_DEFINE(integer_literal)

	const string_literal_type string_literal = "string";
	const auto string_literal_def = x3::lexeme['"' > *(x3::char_ - (x3::lit('"') | '\n' | '\r')) > '"'];
	BOOST_SPIRIT_DEFINE(string_literal)

	const boolean_literal_type boolean_literal = "boolean literal";
	const auto boolean_literal_def = x3::lexeme[symbols::rf::booleans >> not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(boolean_literal)

	const rarity_literal_type rarity_literal = "rarity literal";
	const auto rarity_literal_def = x3::lexeme[symbols::rf::rarities >> not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(rarity_literal)

	const shape_literal_type shape_literal = "shape literal";
	const auto shape_literal_def = x3::lexeme[symbols::rf::shapes >> not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(shape_literal)

	const suit_literal_type suit_literal = "suit literal";
	const auto suit_literal_def = x3::lexeme[symbols::rf::suits >> not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(suit_literal)

	const influence_literal_type influence_literal = "influence literal";
	const auto influence_literal_def = x3::lexeme[symbols::rf::influences >> not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(influence_literal)

	// ---- rules ----

	const comparison_operator_expression_type comparison_operator_expression = "comparison operator";
	const auto comparison_operator_expression_def = symbols::rf::comparison_operators | x3::attr(lang::comparison_type::equal);
	BOOST_SPIRIT_DEFINE(comparison_operator_expression)

	const exact_matching_policy_expression_type exact_matching_policy_expression = "exact matching policy expression";
	const auto exact_matching_policy_expression_def = ("==" > x3::attr(true)) | x3::attr(false);
	BOOST_SPIRIT_DEFINE(exact_matching_policy_expression)

	const visibility_statement_type visibility_statement = "visibility statement";
	const auto visibility_statement_def = x3::lexeme[symbols::rf::visibility_literals >> not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(visibility_statement)
} // namespace common

namespace sf
{
	// ---- literal types ----

	const floating_point_literal_type floating_point_literal = "number (fractional)";
	const auto floating_point_literal_def = x3::real_parser<double, x3::strict_real_policies<double>>{};
	BOOST_SPIRIT_DEFINE(floating_point_literal)

	const underscore_literal_type underscore_literal = "underscore literal";
	const auto underscore_literal_def = x3::lexeme['_' >> common::not_alnum_or_underscore] > x3::attr(ast::sf::underscore_literal());
	BOOST_SPIRIT_DEFINE(underscore_literal)

	// ---- expressions ----

	// moved here due to circular dependency
	const action_type action = "action";

	const compound_action_expression_type compound_action_expression = "compound action expression";
	const auto compound_action_expression_def = '{' >> *action > '}';
	BOOST_SPIRIT_DEFINE(compound_action_expression)

	const literal_expression_type literal_expression = "literal";
	const auto literal_expression_def =
	// order here is crucial in context-sensitive grammars
	// this grammar is context-free but the order will affect performance
	// we can clearly assume that integers and strings will be the most popular
	// note: order should match types in literal_expression_type::attribute_type
		  floating_point_literal
		| common::integer_literal
		| common::string_literal
		| common::boolean_literal
		| common::rarity_literal
		| common::shape_literal
		| common::suit_literal
		| common::influence_literal
		| underscore_literal;
	BOOST_SPIRIT_DEFINE(literal_expression)

	// moved here due to circular dependency
	const value_expression_type value_expression = "expression";

	const value_expression_sequence_type value_expression_sequence = "expression list";
	const auto value_expression_sequence_def = (value_expression % ',') | x3::attr(ast::sf::value_expression_sequence());
	BOOST_SPIRIT_DEFINE(value_expression_sequence)

	const price_range_query_type price_range_query = "price range query";
	const auto price_range_query_def = '$' > common::identifier > '(' > value_expression_sequence > ')';
	BOOST_SPIRIT_DEFINE(price_range_query)

	const auto value_expression_def =
		  compound_action_expression
		| literal_expression
		| common::identifier
		| price_range_query;
	BOOST_SPIRIT_DEFINE(value_expression)

	// ---- definitions ----

	const constant_definition_type constant_definition = "constant definiton";
	const auto constant_definition_def = common::identifier >> '=' > value_expression;
	BOOST_SPIRIT_DEFINE(constant_definition)

	// future space for metedata definitions
	const definition_type definition = "definition";
	const auto definition_def = constant_definition;
	BOOST_SPIRIT_DEFINE(definition)

	// ---- rules ----

	const comparison_condition_type comparison_condition = "comparison condition";
	const auto comparison_condition_def =
		x3::lexeme[symbols::sf::comparison_condition_properties >> common::not_alnum_or_underscore]
		> common::comparison_operator_expression
		> value_expression;
	BOOST_SPIRIT_DEFINE(comparison_condition)

	const array_condition_type array_condition = "array condition";
	const auto array_condition_def =
		x3::lexeme[symbols::sf::array_condition_properties >> common::not_alnum_or_underscore]
		> common::exact_matching_policy_expression
		> value_expression;
	BOOST_SPIRIT_DEFINE(array_condition)

	const boolean_condition_type boolean_condition = "boolean condition";
	const auto boolean_condition_def =
		x3::lexeme[symbols::rf::boolean_condition_properties >> common::not_alnum_or_underscore]
		> value_expression;
	BOOST_SPIRIT_DEFINE(boolean_condition)

	const socket_group_condition_type socket_group_condition = "socket group condition";
	const auto socket_group_condition_def =
		x3::lexeme[lang::keywords::rf::socket_group >> common::not_alnum_or_underscore]
		> value_expression;
	BOOST_SPIRIT_DEFINE(socket_group_condition)

	const condition_type condition = "condition";
	const auto condition_def =
		  comparison_condition
		| array_condition
		| boolean_condition
		| socket_group_condition;
	BOOST_SPIRIT_DEFINE(condition)

	const unary_action_type unary_action = "unary action";
	const auto unary_action_def =
		x3::lexeme[symbols::sf::unary_action_types >> common::not_alnum_or_underscore]
		> value_expression;
	BOOST_SPIRIT_DEFINE(unary_action)

	const compound_action_type compound_action = "compound action";
	const auto compound_action_def =
		x3::lexeme["Set" >> common::not_alnum_or_underscore]
		> value_expression;
	BOOST_SPIRIT_DEFINE(compound_action)

	const auto action_def = compound_action | unary_action;
	BOOST_SPIRIT_DEFINE(action)

	// ---- filter structure ----

	// moved here due to circular dependency
	const rule_block_type rule_block = "rule block";

	const statement_type statement = "statement";
	const auto statement_def = action | common::visibility_statement | rule_block;
	BOOST_SPIRIT_DEFINE(statement)

	const auto rule_block_def = *condition >> x3::lit('{') > *statement > x3::lit('}');
	BOOST_SPIRIT_DEFINE(rule_block)

	const filter_structure_type filter_structure = "filter structure";
	const auto filter_structure_def = *definition > *statement;
	BOOST_SPIRIT_DEFINE(filter_structure)

	const grammar_type grammar = "code";
	const auto grammar_def = filter_structure > x3::eoi;
	BOOST_SPIRIT_DEFINE(grammar)

} // namespace sf

namespace rf
{
	// ---- literal types ----

	const color_literal_type color_literal = "color literal";
	const auto color_literal_def =
		  common::integer_literal
		> common::integer_literal
		> common::integer_literal
		> -common::integer_literal;
	BOOST_SPIRIT_DEFINE(color_literal)

	const icon_literal_type icon_literal = "icon literal";
	const auto icon_literal_def =
		common::integer_literal
		> x3::lexeme[common::suit_literal >> common::not_alnum_or_underscore]
		> x3::lexeme[common::shape_literal >> common::not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(icon_literal)

	const string_literal_array_type string_literal_array = "1 or more string literals";
	const auto string_literal_array_def = +common::string_literal;
	BOOST_SPIRIT_DEFINE(string_literal_array)

	const influence_literal_array_type influence_literal_array = "1 or more influence literals";
	const auto influence_literal_array_def = +x3::lexeme[common::influence_literal >> common::not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(influence_literal_array)

	// ---- conditions ----

	const rarity_condition_type rarity_condition = "rarity condition";
	const auto rarity_condition_def =
		x3::lexeme[lang::keywords::rf::rarity >> common::not_alnum_or_underscore]
		> common::comparison_operator_expression
		> common::rarity_literal;
	BOOST_SPIRIT_DEFINE(rarity_condition)

	const numeric_condition_type numeric_condition = "numeric condition";
	const auto numeric_condition_def =
		x3::lexeme[symbols::rf::numeric_comparison_condition_properties >> common::not_alnum_or_underscore]
		> common::comparison_operator_expression
		> common::integer_literal;
	BOOST_SPIRIT_DEFINE(numeric_condition)

	const string_array_condition_type string_array_condition = "string array condition";
	const auto string_array_condition_def =
		x3::lexeme[symbols::rf::string_array_condition_properties >> common::not_alnum_or_underscore]
		> common::exact_matching_policy_expression
		> string_literal_array;
	BOOST_SPIRIT_DEFINE(string_array_condition)

	const has_influence_condition_type has_influence_condition = "has influence condition";
	const auto has_influence_condition_def =
		x3::lexeme[lang::keywords::rf::has_influence >> common::not_alnum_or_underscore]
		> common::exact_matching_policy_expression
		> influence_literal_array;
	BOOST_SPIRIT_DEFINE(has_influence_condition)

	const socket_group_condition_type socket_group_condition = "socket group condition";
	const auto socket_group_condition_def =
		x3::lexeme[lang::keywords::rf::socket_group >> common::not_alnum_or_underscore]
		> common::identifier;
	BOOST_SPIRIT_DEFINE(socket_group_condition)

	const boolean_condition_type boolean_condition = "boolean condition";
	const auto boolean_condition_def =
		x3::lexeme[symbols::rf::boolean_condition_properties >> common::not_alnum_or_underscore]
		> common::boolean_literal;
	BOOST_SPIRIT_DEFINE(boolean_condition)

	const condition_type condition = "condition";
	const auto condition_def =
		rarity_condition
		| numeric_condition
		| string_array_condition
		| has_influence_condition
		| socket_group_condition
		| boolean_condition;
	BOOST_SPIRIT_DEFINE(condition)

	// ---- actions ----

	const color_action_type color_action = "color action";
	const auto color_action_def =
		x3::lexeme[symbols::rf::color_actions >> common::not_alnum_or_underscore]
		> color_literal;
	BOOST_SPIRIT_DEFINE(color_action)

	const set_font_size_action_type set_font_size_action = "font size action";
	const auto set_font_size_action_def =
		x3::lexeme[lang::keywords::rf::set_font_size >> common::not_alnum_or_underscore]
		> common::integer_literal;
	BOOST_SPIRIT_DEFINE(set_font_size_action)

	const play_alert_sound_action_type play_alert_sound_action = "alert sound action";
	const auto play_alert_sound_action_def =
		x3::lexeme[lang::keywords::rf::play_alert_sound >> common::not_alnum_or_underscore]
		> common::integer_literal   // sound ID
		> -common::integer_literal; // volume (optional token)
	BOOST_SPIRIT_DEFINE(play_alert_sound_action)

	const play_alert_sound_positional_action_type play_alert_sound_positional_action = "positional alert sound action";
	const auto play_alert_sound_positional_action_def =
		x3::lexeme[lang::keywords::rf::play_alert_sound_positional >> common::not_alnum_or_underscore]
		> common::integer_literal   // sound ID
		> -common::integer_literal; // volume (optional token)
	BOOST_SPIRIT_DEFINE(play_alert_sound_positional_action)

	const custom_alert_sound_action_type custom_alert_sound_action = "custom alert sound action";
	const auto custom_alert_sound_action_def =
		x3::lexeme[lang::keywords::rf::custom_alert_sound >> common::not_alnum_or_underscore]
		> common::string_literal;
	BOOST_SPIRIT_DEFINE(custom_alert_sound_action)

	const disable_drop_sound_action_type disable_drop_sound_action = "disable drop sound action";
	const auto disable_drop_sound_action_def = x3::lexeme[lang::keywords::rf::disable_drop_sound >> common::not_alnum_or_underscore];
	BOOST_SPIRIT_DEFINE(disable_drop_sound_action)

	const minimap_icon_action_type minimap_icon_action = "minimap icon action";
	const auto minimap_icon_action_def =
		x3::lexeme[lang::keywords::rf::minimap_icon >> common::not_alnum_or_underscore]
		> icon_literal;
	BOOST_SPIRIT_DEFINE(minimap_icon_action)

	const play_effect_action_type play_effect_action = "play effct action";
	const auto play_effect_action_def =
		x3::lexeme[lang::keywords::rf::play_effect >> common::not_alnum_or_underscore]
		> common::suit_literal
		> ((x3::lexeme[lang::keywords::rf::temp >> common::not_alnum_or_underscore] > x3::attr(true)) | x3::attr(false));
	BOOST_SPIRIT_DEFINE(play_effect_action)

	const action_type action = "action";
	const auto action_def =
		color_action
		| set_font_size_action
		| play_alert_sound_action
		| play_alert_sound_positional_action
		| custom_alert_sound_action
		| disable_drop_sound_action
		| minimap_icon_action
		| play_effect_action;
	BOOST_SPIRIT_DEFINE(action)

	// ---- filter structure ----

	const rule_type rule = "rule";
	const auto rule_def = condition | action;
	BOOST_SPIRIT_DEFINE(rule)

	const filter_block_type filter_block = "filter block";
	const auto filter_block_def = common::visibility_statement > *rule;
	BOOST_SPIRIT_DEFINE(filter_block)

	const grammar_type grammar = "code";
	const auto grammar_def = *filter_block;
	BOOST_SPIRIT_DEFINE(grammar)
} // namespace rf

} // namespace fs::parser::detail
