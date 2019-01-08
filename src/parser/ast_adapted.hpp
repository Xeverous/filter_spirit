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
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/for_each.hpp>
// uncomment if adding any such members
// #include <boost/fusion/include/std_pair.hpp>
// #include <boost/fusion/include/std_array.hpp>
// #include <boost/fusion/include/std_tuple.hpp>

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
	fs::ast::string_literal,
	value
)

BOOST_FUSION_ADAPT_STRUCT(
	fs::ast::color_literal,
	r, g, b, a
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
	fs::ast::code_line,
	value
)

