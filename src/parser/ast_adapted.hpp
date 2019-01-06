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
#include <variant> // std::visit

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
    fs::ast::identifier,
    value
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
    fs::ast::code_line,
    value
)

namespace fs::ast
{



}
