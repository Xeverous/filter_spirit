/**
 * @file AST adaptations
 *
 * @brief This file is intended only for BOOST_FUSION_ADAPT_STRUCT macros
 *
 * @details adaptations must be in global scope
 */
#pragma once
#include "parser/ast.hpp"
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

// core tokens

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::boolean_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::integer_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::opacity_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rarity_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::shape_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::suit_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::color_literal,
	r, g, b, a
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::group_literal,
	r, g, b, w
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::identifier,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::string_literal,
	value
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::object_type_expression,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::array_type_expression,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::array_expression,
	values
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::comparison_operator_expression,
	comparison
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::item_level_condition,
	comparison, level
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::drop_level_condition,
	comparison, level
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::visibility_action,
	show
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::border_color_action,
	color
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::text_color_action,
	color
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::background_color_action,
	color
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::condition_list,
	condition_expressions
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::action_list,
	action_expressions
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::constant_definition,
	type, name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::constant_definition_list,
	constant_definitions
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rule_block,
	conditions, actions, nested_blocks
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::rule_block_list,
	blocks
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::parser::ast::filter_specification,
	constants_list, actions, blocks
)

