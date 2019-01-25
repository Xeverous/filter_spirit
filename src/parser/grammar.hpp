/**
 * @file grammar type definitions
 *
 * @details This file is intended for main parser API.
 * This is the file that all parser source files should include.
 * Use BOOST_SPIRIT_DECLARE here.
 */
#pragma once
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "print/generic.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <string>

namespace fs::parser
{

// Inherit rule ID type from these type if you want Spirit to call on_success/on_error
// this allows the same things as expr[func] in grammar definitions but does not bloat it.
// This type can be used for any rule that has x3::position_tagged attribute type - it just
// fills it with source code information (eg for later semantic analysis)
struct annotate_on_success
{
	template <typename Iterator, typename Context, typename... Types>
	void on_success(
		const Iterator& first,
		const Iterator& last,
		x3::variant<Types...>& ast,
		const Context& context)
	{
		ast.apply_visitor(x3::make_lambda_visitor<void>([&](auto& node)
		{
			this->on_success(first, last, node, context);
		}));
	}

	template <typename T, typename Iterator, typename Context>
	void on_success(
		const Iterator& first,
		const Iterator& last,
		T& ast,
		const Context& context)
	{
		position_cache_type& positions = x3::get<position_cache_tag>(context).get();
		positions.annotate(ast, first, last);
	}
};

struct error_on_error
{
	template <typename Iterator, typename Exception, typename Context>
	x3::error_handler_result on_error(
		Iterator& /* first */,
		const Iterator& /* last */,
		const Exception& ex,
		const Context& context)
	{
		const position_cache_type& positions = x3::get<position_cache_tag>(context).get();
		const iterator_type error_first = ex.where();
		const auto error_last = ++iterator_type(error_first);
		print::print_line_number_with_indication_and_texts(
			x3::get<error_stream_tag>(context).get(),
			range_type(positions.first(), positions.last()),
			range_type(error_first, error_last),
			"parse error: expected '",
			ex.which(),
			"' here");
		return x3::error_handler_result::fail;
	}
};


// rule IDs
// use multiple inheritance to add more handlers
// rules which do not have any handlers can use forward declared types
struct comment_class                      : error_on_error, annotate_on_success {};

struct boolean_class                      : error_on_error, annotate_on_success {};
struct integer_class                      : error_on_error, annotate_on_success {};
struct opacity_class                      : error_on_error, annotate_on_success {};
struct rarity_literal_class               : error_on_error, annotate_on_success {};
struct shape_literal_class                : error_on_error, annotate_on_success {};
struct suit_literal_class                 : error_on_error, annotate_on_success {};
struct color_literal_class                : error_on_error, annotate_on_success {};
struct group_literal_class                : error_on_error, annotate_on_success {};
struct group_literal_impl_class           : error_on_error, annotate_on_success {};
struct identifier_impl_class;
struct identifier_class                   : error_on_error, annotate_on_success {};
struct string_literal_class               : error_on_error, annotate_on_success {};

struct object_type_expression_class       : error_on_error, annotate_on_success {};
struct value_expression_class             : error_on_error, annotate_on_success {};
struct constant_definition_class          : error_on_error, annotate_on_success {};

struct code_line_class                    : error_on_error, annotate_on_success {};

struct grammar_class                      : error_on_error, annotate_on_success {};

// whitespace_type should be defined here but it has been moved to parser/config.hpp for
// dependency reasons. See config.hpp for details.
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

}

namespace fs
{

parser::grammar_type grammar();
parser::skipper_type skipper();

}
