/**
 * @file grammar type definitions
 *
 * @details This file is intended for main parser API.
 * This is the file that all parser source files should include.
 * Use BOOST_SPIRIT_DECLARE here.
 */
#pragma once

#include <fs/parser/ast.hpp>
#include <fs/parser/error.hpp>
#include <fs/parser/detail/config.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

// workaround for https://github.com/boostorg/spirit/issues/461
// or maybe the missing implementation of the most obvious substitution
template <typename T>
struct boost::spirit::x3::traits::is_substitute<T, T> : boost::mpl::true_ {};

namespace fs::parser::detail
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
		Iterator& first,
		const Iterator& /* last */,
		const Exception& ex, // x3::expectation_failure<Iterator> or similar type
		const Context& context)
	{
		error_holder_type& error_holder = x3::get<error_holder_tag>(context).get();
		error_holder.push_back(parse_error{ex.where(), first, ex.which()});
		return x3::error_handler_result::fail;
	}
};


// rule IDs
// use multiple inheritance to add more handlers
// rules which do not have any handlers can use forward declared types

// ---- whitespace ----

// (nothing - whitespace does not synthesize any attributes)

// ---- fundamental tokens ----

struct identifier_class                      : error_on_error, annotate_on_success {};

// ---- version requirement ----

struct version_literal_class                 : error_on_error, annotate_on_success {};
struct version_requirement_statement_class   : error_on_error, annotate_on_success {};

// ---- config ----

struct config_param_class                    : error_on_error, annotate_on_success {};
struct config_class                          : error_on_error, annotate_on_success {};

// ---- literal types ----

struct floating_point_literal_class          : error_on_error, annotate_on_success {};
struct integer_literal_class                 : error_on_error, annotate_on_success {};
struct string_literal_class                  : error_on_error, annotate_on_success {};
struct boolean_literal_class                 : error_on_error, annotate_on_success {};
struct rarity_literal_class                  : error_on_error, annotate_on_success {};
struct shape_literal_class                   : error_on_error, annotate_on_success {};
struct suit_literal_class                    : error_on_error, annotate_on_success {};

// ---- expressions ----

struct literal_expression_class              : error_on_error, annotate_on_success {};
struct value_expression_list_class           : error_on_error, annotate_on_success {};
struct function_call_class                   : error_on_error, annotate_on_success {};
struct price_range_query_class               : error_on_error, annotate_on_success {};
struct array_expression_class                : error_on_error, annotate_on_success {};
struct primary_expression_class              : error_on_error, annotate_on_success {};
struct subscript_class                       : error_on_error, annotate_on_success {};
struct postfix_expression_class              : error_on_error, annotate_on_success {};
struct value_expression_class                : error_on_error, annotate_on_success {};

// ---- definitions ----

struct constant_definition_class             : error_on_error, annotate_on_success {};

// ---- rules ----

struct comparison_operator_expression_class  : error_on_error, annotate_on_success {};
struct comparison_condition_class            : error_on_error, annotate_on_success {};
struct exact_matching_policy_operator_class  : error_on_error, annotate_on_success {};
struct string_condition_class                : error_on_error, annotate_on_success {};
struct boolean_condition_class               : error_on_error, annotate_on_success {};
struct socket_group_condition_class          : error_on_error, annotate_on_success {};
struct condition_class                       : error_on_error, annotate_on_success {};
struct unary_action_class                    : error_on_error, annotate_on_success {};
struct action_class                          : error_on_error, annotate_on_success {};

// ---- filter structure ----

struct visibility_statement_class            : error_on_error, annotate_on_success {};
struct statement_class                       : error_on_error, annotate_on_success {};
struct rule_block_class                      : error_on_error, annotate_on_success {};
struct filter_structure_class                : error_on_error, annotate_on_success {};

//

struct color_expression_class                : error_on_error, annotate_on_success {};
struct border_color_action_class             : error_on_error, annotate_on_success {};
struct text_color_action_class               : error_on_error, annotate_on_success {};
struct background_color_action_class         : error_on_error, annotate_on_success {};

struct condition_list_class                  : error_on_error, annotate_on_success {};
struct action_list_class                     : error_on_error, annotate_on_success {};

struct rule_block_list_class                 : error_on_error, annotate_on_success {};

struct grammar_class                         : error_on_error, annotate_on_success {};

// ---- whitespace ----

// whitespace_type should be defined here but it has been moved to parser/config.hpp for
// dependency reasons. See config.hpp for details.
BOOST_SPIRIT_DECLARE(whitespace_type)

// all comments are ignored
using comment_type = x3::rule<struct comment_class /*, intentionally nothing */>;
BOOST_SPIRIT_DECLARE(comment_type)

// ---- fundamental tokens ----

// identifier has an extra intermediate rule because Spirit for (?) it's container detection reasons
// can not match identifier grammar with a struct that contains only std::string (compiles only with std::string directly)
// to workaround, we just add 1 more step with the same grammar
// https://stackoverflow.com/questions/18166958
using identifier_impl_type = x3::rule<struct identifier_impl_class, std::string>;
BOOST_SPIRIT_DECLARE(identifier_impl_type)
using identifier_type = x3::rule<identifier_class, ast::identifier>;
BOOST_SPIRIT_DECLARE(identifier_type)
using not_alnum_or_underscore_type = x3::rule<struct not_alnum_or_underscore_class /*, intentionally nothing */>;
BOOST_SPIRIT_DECLARE(not_alnum_or_underscore_type)

// ---- version requirement ----

using version_literal_type = x3::rule<version_literal_class, ast::version_literal>;
BOOST_SPIRIT_DECLARE(version_literal_type)

using version_requirement_statement_type = x3::rule<version_requirement_statement_class, ast::version_literal>;
BOOST_SPIRIT_DECLARE(version_requirement_statement_type)

// ---- config ----

using config_param_type = x3::rule<config_param_class, ast::config_param>;
BOOST_SPIRIT_DECLARE(config_param_type)

using config_type = x3::rule<config_class, ast::config>;
BOOST_SPIRIT_DECLARE(config_type)

// ---- literal types ----

using floating_point_literal_type = x3::rule<floating_point_literal_class, ast::floating_point_literal>;
BOOST_SPIRIT_DECLARE(floating_point_literal_type)

using integer_literal_type = x3::rule<integer_literal_class, ast::integer_literal>;
BOOST_SPIRIT_DECLARE(integer_literal_type)

using string_literal_type = x3::rule<string_literal_class, ast::string_literal>;
BOOST_SPIRIT_DECLARE(string_literal_type)

using boolean_literal_type = x3::rule<boolean_literal_class, ast::boolean_literal>;
BOOST_SPIRIT_DECLARE(boolean_literal_type)

using rarity_literal_type = x3::rule<rarity_literal_class, ast::rarity_literal>;
BOOST_SPIRIT_DECLARE(rarity_literal_type)

using shape_literal_type = x3::rule<shape_literal_class, ast::shape_literal>;
BOOST_SPIRIT_DECLARE(shape_literal_type)

using suit_literal_type = x3::rule<suit_literal_class, ast::suit_literal>;
BOOST_SPIRIT_DECLARE(suit_literal_type)

// ---- expressions ----

using literal_expression_type = x3::rule<literal_expression_class, ast::literal_expression>;
BOOST_SPIRIT_DECLARE(literal_expression_type)

using value_expression_list_type = x3::rule<value_expression_list_class, ast::value_expression_list>;
BOOST_SPIRIT_DECLARE(value_expression_list_type)

using function_call_type = x3::rule<function_call_class, ast::function_call>;
BOOST_SPIRIT_DECLARE(function_call_type)

using price_range_query_type = x3::rule<price_range_query_class, ast::price_range_query>;
BOOST_SPIRIT_DECLARE(price_range_query_type)

using array_expression_type = x3::rule<array_expression_class, ast::array_expression>;
BOOST_SPIRIT_DECLARE(array_expression_type)

using primary_expression_type = x3::rule<primary_expression_class, ast::primary_expression>;
BOOST_SPIRIT_DECLARE(primary_expression_type)

using subscript_type = x3::rule<subscript_class, ast::subscript>;
BOOST_SPIRIT_DECLARE(subscript_type)

using postfix_expression_type = x3::rule<postfix_expression_class, ast::postfix_expression>;
BOOST_SPIRIT_DECLARE(postfix_expression_type)

using value_expression_type = x3::rule<value_expression_class, ast::value_expression>;
BOOST_SPIRIT_DECLARE(value_expression_type)

// ---- definitions ----

using constant_definition_type = x3::rule<constant_definition_class, ast::constant_definition>;
BOOST_SPIRIT_DECLARE(constant_definition_type)

// ---- rules ----

using comparison_operator_expression_type = x3::rule<comparison_operator_expression_class, ast::comparison_operator_expression>;
BOOST_SPIRIT_DECLARE(comparison_operator_expression_type)

using comparison_condition_type = x3::rule<comparison_condition_class, ast::comparison_condition>;
BOOST_SPIRIT_DECLARE(comparison_condition_type)

using exact_matching_policy_operator_type = x3::rule<exact_matching_policy_operator_class, ast::exact_matching_policy>;
BOOST_SPIRIT_DECLARE(exact_matching_policy_operator_type)

using string_condition_type = x3::rule<string_condition_class, ast::string_condition>;
BOOST_SPIRIT_DECLARE(string_condition_type)

using boolean_condition_type = x3::rule<boolean_condition_class, ast::boolean_condition>;
BOOST_SPIRIT_DECLARE(boolean_condition_type)

using socket_group_condition_type = x3::rule<socket_group_condition_class, ast::socket_group_condition>;
BOOST_SPIRIT_DECLARE(socket_group_condition_type)

using condition_type = x3::rule<condition_class, ast::condition>;
BOOST_SPIRIT_DECLARE(condition_type)

using unary_action_type = x3::rule<unary_action_class, ast::unary_action>;
BOOST_SPIRIT_DECLARE(unary_action_type)

using action_type = x3::rule<action_class, ast::action>;
BOOST_SPIRIT_DECLARE(action_type)

// ---- filter structure ----

using visibility_statement_type = x3::rule<visibility_statement_class, ast::visibility_statement>;
BOOST_SPIRIT_DECLARE(visibility_statement_type)

using statement_type = x3::rule<statement_class, ast::statement>;
BOOST_SPIRIT_DECLARE(statement_type)

using rule_block_type = x3::rule<rule_block_class, ast::rule_block>;
BOOST_SPIRIT_DECLARE(rule_block_type)

using filter_structure_type = x3::rule<filter_structure_class, ast::filter_structure>;
BOOST_SPIRIT_DECLARE(filter_structure_type)

using grammar_type = x3::rule<grammar_class, ast::ast_type>;
BOOST_SPIRIT_DECLARE(grammar_type)

}

namespace fs::parser
{

detail::grammar_type grammar();
detail::skipper_type skipper();

}
