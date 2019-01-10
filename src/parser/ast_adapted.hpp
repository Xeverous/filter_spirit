/**
 * @file AST adaptations
 *
 * @brief This file is intended only for BOOST_FUSION_ADAPT_STRUCT macros
 *
 * @details adaptations must be in global scope
 */
#pragma once

#include "ast.hpp"
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
	fs::ast::boolean,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::integer,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::opacity,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::identifier,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::rarity_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::shape_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::suit_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::color_literal,
	r, g, b, a
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::group_literal,
	r, g, b, w
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::string_literal,
	value
)

// ----

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_boolean_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_number_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_level_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_sound_id_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_volume_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_rarity_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_shape_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_suit_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_color_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_group_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::constant_string_definition,
	name, value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::code_line,
	value
)

