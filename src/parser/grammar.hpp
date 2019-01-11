/**
 * @file grammar type definitions
 *
 * @details This file is intended for main parser API.
 * This is the file that all parser source files should include.
 * Use BOOST_SPIRIT_DECLARE here.
 */
#pragma once
#include "ast.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;

// tag used to get the position cache from the context
struct position_cache_tag;

// Inherit rule ID type from this type if you want Spirit to call on_success/on_error
// this allows the same things as expr[func] in grammar definitions but does not bloat it.
// This type can be used for any rule that has x3::position_tagged attribute type - it just
// fills it with source code information (eg for later semantic analysis)
struct error_handler
{
	template <typename Iterator, typename Exception, typename Context>
	x3::error_handler_result on_error(
		Iterator& /* first */,
		const Iterator& /* last */,
		const Exception& ex,
		const Context& context)
	{
		auto& error_handler = x3::get<x3::error_handler_tag>(context).get();
		std::string message = "parse error: expected: '" + ex.which() + "' here";
		error_handler(ex.where(), message);
		return x3::error_handler_result::fail;
	}
};


// rule IDs
// use multiple inheritance to add more handlers
// rules which do not have any handlers can use forward declared types
struct comment_class                      : error_handler, x3::annotate_on_success {};

struct boolean_class                      : error_handler, x3::annotate_on_success {};
struct integer_class                      : error_handler, x3::annotate_on_success {};
struct opacity_class                      : error_handler, x3::annotate_on_success {};
struct rarity_literal_class               : error_handler, x3::annotate_on_success {};
struct shape_literal_class                : error_handler, x3::annotate_on_success {};
struct suit_literal_class                 : error_handler, x3::annotate_on_success {};
struct color_literal_class                : error_handler, x3::annotate_on_success {};
struct group_literal_class                : error_handler, x3::annotate_on_success {};
struct group_literal_impl_class           : error_handler, x3::annotate_on_success {};
struct identifier_impl_class;
struct identifier_class                   : error_handler, x3::annotate_on_success {};
struct string_literal_class               : error_handler, x3::annotate_on_success {};

struct object_type_expression_class       : error_handler, x3::annotate_on_success {};
struct value_expression_class             : error_handler, x3::annotate_on_success {};
struct constant_definition_class          : error_handler, x3::annotate_on_success {};

struct code_line_class                    : error_handler, x3::annotate_on_success {};

struct grammar_class                      : error_handler, x3::annotate_on_success {};

/*
 * whitespace
 * Filter Spirit grammar skips any whitespace except newline character
 *
 * Spirit constness bug workaround, see:
 * https://stackoverflow.com/a/54095167/4818802
 * https://github.com/boostorg/spirit/pull/347
 */
// using whitespace_type = x3::rule<class whitespace_class>;
using whitespace_type = x3::rule<class whitespace_class, const x3::unused_type>;
BOOST_SPIRIT_DECLARE(whitespace_type)

// comment - a line that starts with #
using comment_type = x3::rule<comment_class>;
BOOST_SPIRIT_DECLARE(comment_type)

using boolean_type = x3::rule<boolean_class, ast::boolean_literal>;
BOOST_SPIRIT_DECLARE(boolean_type)

using integer_type = x3::rule<integer_class, ast::integer_literal>;
BOOST_SPIRIT_DECLARE(integer_type)

using opacity_type = x3::rule<opacity_class, ast::opacity_literal>;
BOOST_SPIRIT_DECLARE(opacity_type)


using rarity_literal_type = x3::rule<rarity_literal_class, ast::rarity_literal>;
BOOST_SPIRIT_DECLARE(rarity_literal_type)

using shape_literal_type = x3::rule<shape_literal_class, ast::shape_literal>;
BOOST_SPIRIT_DECLARE(shape_literal_type)

using suit_literal_type = x3::rule<suit_literal_class, ast::suit_literal>;
BOOST_SPIRIT_DECLARE(suit_literal_type)

using color_literal_type = x3::rule<color_literal_class, ast::color_literal>;
BOOST_SPIRIT_DECLARE(color_literal_type)

// same issue as with integer_literal
using group_literal_impl_type = x3::rule<group_literal_impl_class, ast::group_literal>;
BOOST_SPIRIT_DECLARE(group_literal_impl_type)
using group_literal_type = x3::rule<group_literal_class, ast::group_literal>;
BOOST_SPIRIT_DECLARE(group_literal_type)

// identifier has an extra intermediate rule because Spirit for (?) it's container detection reasons
// can not match identifier grammar with a struct that contains only std::string (compiles only with std::string directly)
// to workaround, we just add 1 more step with the same grammar
// https://stackoverflow.com/questions/18166958
using identifier_impl_type = x3::rule<identifier_impl_class, std::string>;
BOOST_SPIRIT_DECLARE(identifier_impl_type)
using identifier_type = x3::rule<identifier_class, ast::identifier>;
BOOST_SPIRIT_DECLARE(identifier_type)

using string_literal_type = x3::rule<string_literal_class, ast::string_literal>;
BOOST_SPIRIT_DECLARE(string_literal_type)

// ----

using object_type_expression_type = x3::rule<object_type_expression_class, ast::object_type_expression>;
BOOST_SPIRIT_DECLARE(object_type_expression_type)

using value_expression_type = x3::rule<value_expression_class, ast::value_expression>;
BOOST_SPIRIT_DECLARE(value_expression_type)

using constant_definition_type = x3::rule<constant_definition_class, ast::constant_definition>;
BOOST_SPIRIT_DECLARE(constant_definition_type)

// filter language consists of lines, of which every is a comment or empty or some code
using code_line_type = x3::rule<code_line_class, ast::code_line>;
BOOST_SPIRIT_DECLARE(code_line_type)

// the entire language grammar
using grammar_type = x3::rule<grammar_class, std::vector<code_line_type::attribute_type>>;
BOOST_SPIRIT_DECLARE(grammar_type)

using skipper_type = whitespace_type;

// Boost Spirit recommends that this should be in a separate config.hpp file but
// in our case we need skipper type to be visible so we place configuration here
using iterator_type = std::string::const_iterator;
using phrase_context_type = typename x3::phrase_parse_context<skipper_type>::type;
using error_handler_type = boost::spirit::x3::error_handler<iterator_type>;
using context_type = x3::context<x3::error_handler_tag, std::reference_wrapper<error_handler_type>, phrase_context_type>;

}

namespace fs
{

parser::grammar_type grammar();
parser::skipper_type skipper();

}
