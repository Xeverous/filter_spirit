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

// -------- common --------

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::common::comparison_expression,
	operator_, integer)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::common::import_statement,
	path, is_optional)

// -------- real filter --------

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::condition,
	property, comparison, seq)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::action,
	property, seq)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rf::filter_block,
	visibility, rules)

// -------- spirit filter --------

// ---- whitespace ----

// (nothing to adapt)

// ---- fundamental tokens ----

// (nothing to adapt)

// ---- literal types ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::common::socket_spec_literal,
	socket_count, socket_colors)

// ---- expressions ----

// (nothing to adapt)

// ---- definitions ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::constant_definition,
	value_name, value)

// ---- conditions ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::price_comparison_condition,
	comparison, seq)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::official_condition,
	property, comparison, seq)

// ---- actions ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::official_action,
	property, seq)

// ---- filter structure ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::dynamic_visibility_statement,
	policy, seq)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::behavior_statement,
	visibility, continue_)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::unknown_statement,
	name, comparison, seq)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::sf::filter_structure,
	definitions, statements)
