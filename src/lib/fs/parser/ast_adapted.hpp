/**
 * @file AST adaptations
 *
 * @brief This file is intended only for BOOST_FUSION_ADAPT_STRUCT macros
 *
 * @details
 * - adaptations must be in global scope
 * - DO NOT adapt 1-member structs, use inheritance or overload operator= instead
 *   (Spirit is not intended to be used with 1-element fusions)
 * - there are some tricks available (partial adapations, rearranged order of adapted members)
 *   lookup Boost Fusion documentation for more details
 */
#pragma once

#include <fs/parser/ast.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

// uncomment if adapting any such members
// but prefer Boost counterparts
// #include <boost/fusion/include/std_pair.hpp>
// #include <boost/fusion/include/std_array.hpp>
// #include <boost/fusion/include/std_tuple.hpp>

// for pretty-print of AST types
// #include <boost/fusion/include/io.hpp>
// #include <boost/fusion/sequence/io.hpp>
// #include <boost/optional/optional_io.hpp>

// allow Boost Fusion to pretty print AST types
// using boost::fusion::operator<<;
// using boost::fusion::operators::operator<<;

// -------- spirit filter --------

// ---- whitespace ----

// (nothing to adapt)

// ---- fundamental tokens ----

// (nothing to adapt)

// ---- literal types ----

// (nothing to adapt)

// ---- expressions ----

// (nothing to adapt)

// ---- definitions ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::constant_definition,
	name, value)

// ---- rules ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::comparison_condition,
	property, comparison_type, value)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::array_condition,
	property, exact_match, value)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::boolean_condition,
	property, value)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::unary_action,
	action_type, value)

// ---- filter structure ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::rule_block,
	conditions, statements)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::filter_structure,
	definitions, statements)

// -------- real filter --------

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::color_literal,
	r, g, b, a)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::icon_literal,
	size, suit, shape)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::rarity_condition,
	comparison_type, rarity)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::numeric_condition,
	property, comparison_type, integer)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::string_array_condition,
	property, exact_match, string_literals)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::has_influence_condition,
	exact_match, influence_literals)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::boolean_condition,
	property, value)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::color_action,
	action, color)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::play_alert_sound_action,
	sound_id, volume)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::play_alert_sound_positional_action,
	sound_id, volume)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::play_effect_action,
	suit, is_temporary)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::filter_block,
	visibility, rules)
