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

namespace common
{
	struct identifier_class                      : error_on_error, annotate_on_success {};
	struct integer_literal_class                 : error_on_error, annotate_on_success {};
	struct string_literal_class                  : error_on_error, annotate_on_success {};
	struct boolean_literal_class                 : error_on_error, annotate_on_success {};
	struct rarity_literal_class                  : error_on_error, annotate_on_success {};
	struct shape_literal_class                   : error_on_error, annotate_on_success {};
	struct suit_literal_class                    : error_on_error, annotate_on_success {};
	struct influence_literal_class               : error_on_error, annotate_on_success {};

	struct comparison_operator_expression_class  : error_on_error, annotate_on_success {};
	struct exact_matching_policy_expression_class: error_on_error, annotate_on_success {};

	struct visibility_statement_class            : error_on_error, annotate_on_success {};
}

namespace sf
{
	// ---- whitespace ----

	// (nothing - whitespace does not synthesize any attributes)

	// ---- fundamental tokens ----

	// (all in common)

	// ---- literal types ----

	struct floating_point_literal_class          : error_on_error, annotate_on_success {};
	struct none_literal_class                    : error_on_error, annotate_on_success {};

	// ---- expressions ----

	struct compound_action_expression_class      : error_on_error, annotate_on_success {};
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
	struct definition_class                      : error_on_error, annotate_on_success {};

	// ---- rules ----

	struct comparison_condition_class            : error_on_error, annotate_on_success {};
	struct array_condition_class                 : error_on_error, annotate_on_success {};
	struct boolean_condition_class               : error_on_error, annotate_on_success {};
	struct socket_group_condition_class          : error_on_error, annotate_on_success {};
	struct condition_class                       : error_on_error, annotate_on_success {};
	struct compound_action_class                 : error_on_error, annotate_on_success {};
	struct unary_action_class                    : error_on_error, annotate_on_success {};
	struct action_class                          : error_on_error, annotate_on_success {};

	// ---- filter structure ----

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
} // namespace sf

namespace rf
{
	// ---- literal types ----

	struct color_literal_class                   : error_on_error, annotate_on_success {};
	struct icon_literal_class                    : error_on_error, annotate_on_success {};
	struct string_literal_array_class            : error_on_error, annotate_on_success {};
	struct influence_literal_array_class         : error_on_error, annotate_on_success {};

	// ---- conditions ----

	struct rarity_condition_class                : error_on_error, annotate_on_success {};
	struct numeric_condition_class               : error_on_error, annotate_on_success {};
	struct string_array_condition_class          : error_on_error, annotate_on_success {};
	struct has_influence_condition_class         : error_on_error, annotate_on_success {};
	struct socket_group_condition_class          : error_on_error, annotate_on_success {};
	struct boolean_condition_class               : error_on_error, annotate_on_success {};

	// ---- actions ----

	struct color_action_class                    : error_on_error, annotate_on_success {};
	struct set_font_size_action_class            : error_on_error, annotate_on_success {};
	struct play_alert_sound_action_class         : error_on_error, annotate_on_success {};
	struct play_alert_sound_positional_action_class : error_on_error, annotate_on_success {};
	struct custom_alert_sound_action_class       : error_on_error, annotate_on_success {};
	struct disable_drop_sound_action_class       : error_on_error, annotate_on_success {};
	struct minimap_icon_action_class             : error_on_error, annotate_on_success {};
	struct play_effect_action_class              : error_on_error, annotate_on_success {};

	// ---- filter structure ----

	struct condition_class                       : error_on_error, annotate_on_success {};
	struct action_class                          : error_on_error, annotate_on_success {};
	struct rule_class                            : error_on_error, annotate_on_success {};
	struct visibility_statement_class            : error_on_error, annotate_on_success {};
	struct filter_block_class                    : error_on_error, annotate_on_success {};
	struct grammar_class                         : error_on_error, annotate_on_success {};
} // namespace rf

namespace common
{
	// ---- whitespace ----

	// whitespace_type should be defined here but it has been moved to parser/config.hpp for
	// dependency reasons. See config.hpp for details.
	BOOST_SPIRIT_DECLARE(whitespace_type)

	// ---- fundamental tokens ----

	// all comments are ignored
	using comment_type = x3::rule<struct comment_class /*, intentionally nothing */>;
	BOOST_SPIRIT_DECLARE(comment_type)

	// identifier has an extra intermediate rule because Spirit for (?) it's container detection reasons
	// can not match identifier grammar with a struct that contains only std::string (compiles only with std::string directly)
	// to workaround, we just add 1 more step with the same grammar
	// https://stackoverflow.com/questions/18166958
	using identifier_impl_type = x3::rule<struct identifier_impl_class, std::string>;
	BOOST_SPIRIT_DECLARE(identifier_impl_type)
	using identifier_type = x3::rule<identifier_class, ast::common::identifier>;
	BOOST_SPIRIT_DECLARE(identifier_type)
	using not_alnum_or_underscore_type = x3::rule<struct not_alnum_or_underscore_class /*, intentionally nothing */>;
	BOOST_SPIRIT_DECLARE(not_alnum_or_underscore_type)

	// ---- literal types ----

	using integer_literal_type = x3::rule<integer_literal_class, ast::common::integer_literal>;
	BOOST_SPIRIT_DECLARE(integer_literal_type)

	using string_literal_type = x3::rule<string_literal_class, ast::common::string_literal>;
	BOOST_SPIRIT_DECLARE(string_literal_type)

	using boolean_literal_type = x3::rule<boolean_literal_class, ast::common::boolean_literal>;
	BOOST_SPIRIT_DECLARE(boolean_literal_type)

	using rarity_literal_type = x3::rule<rarity_literal_class, ast::common::rarity_literal>;
	BOOST_SPIRIT_DECLARE(rarity_literal_type)

	using shape_literal_type = x3::rule<shape_literal_class, ast::common::shape_literal>;
	BOOST_SPIRIT_DECLARE(shape_literal_type)

	using suit_literal_type = x3::rule<suit_literal_class, ast::common::suit_literal>;
	BOOST_SPIRIT_DECLARE(suit_literal_type)

	using influence_literal_type = x3::rule<influence_literal_class, ast::common::influence_literal>;
	BOOST_SPIRIT_DECLARE(influence_literal_type)

	// ---- rules ----

	using comparison_operator_expression_type = x3::rule<comparison_operator_expression_class, ast::common::comparison_operator_expression>;
	BOOST_SPIRIT_DECLARE(comparison_operator_expression_type)

	using exact_matching_policy_expression_type = x3::rule<exact_matching_policy_expression_class, ast::common::exact_matching_policy>;
	BOOST_SPIRIT_DECLARE(exact_matching_policy_expression_type)

	// ---- filter structure ----

	using visibility_statement_type = x3::rule<visibility_statement_class, ast::common::visibility_statement>;
	BOOST_SPIRIT_DECLARE(visibility_statement_type)
} // namespace common

namespace sf
{
	// ---- whitespace ----

	// (all in common)

	// ---- fundamental tokens ----

	// (all in common)

	// ---- literal types ----

	using floating_point_literal_type = x3::rule<floating_point_literal_class, ast::sf::floating_point_literal>;
	BOOST_SPIRIT_DECLARE(floating_point_literal_type)

	using none_literal_type = x3::rule<none_literal_class, ast::sf::none_literal>;
	BOOST_SPIRIT_DECLARE(none_literal_type)

	// ---- expressions ----

	using compound_action_expression_type = x3::rule<compound_action_expression_class, ast::sf::compound_action_expression>;
	BOOST_SPIRIT_DECLARE(compound_action_expression_type)

	using literal_expression_type = x3::rule<literal_expression_class, ast::sf::literal_expression>;
	BOOST_SPIRIT_DECLARE(literal_expression_type)

	using value_expression_list_type = x3::rule<value_expression_list_class, ast::sf::value_expression_list>;
	BOOST_SPIRIT_DECLARE(value_expression_list_type)

	using function_call_type = x3::rule<function_call_class, ast::sf::function_call>;
	BOOST_SPIRIT_DECLARE(function_call_type)

	using price_range_query_type = x3::rule<price_range_query_class, ast::sf::price_range_query>;
	BOOST_SPIRIT_DECLARE(price_range_query_type)

	using array_expression_type = x3::rule<array_expression_class, ast::sf::array_expression>;
	BOOST_SPIRIT_DECLARE(array_expression_type)

	using primary_expression_type = x3::rule<primary_expression_class, ast::sf::primary_expression>;
	BOOST_SPIRIT_DECLARE(primary_expression_type)

	using subscript_type = x3::rule<subscript_class, ast::sf::subscript>;
	BOOST_SPIRIT_DECLARE(subscript_type)

	using postfix_expression_type = x3::rule<postfix_expression_class, ast::sf::postfix_expression>;
	BOOST_SPIRIT_DECLARE(postfix_expression_type)

	using value_expression_type = x3::rule<value_expression_class, ast::sf::value_expression>;
	BOOST_SPIRIT_DECLARE(value_expression_type)

	// ---- definitions ----

	using constant_definition_type = x3::rule<constant_definition_class, ast::sf::constant_definition>;
	BOOST_SPIRIT_DECLARE(constant_definition_type)

	using definition_type = x3::rule<definition_class, ast::sf::definition>;
	BOOST_SPIRIT_DECLARE(definition_type)

	// ---- rules ----

	using comparison_condition_type = x3::rule<comparison_condition_class, ast::sf::comparison_condition>;
	BOOST_SPIRIT_DECLARE(comparison_condition_type)

	using array_condition_type = x3::rule<array_condition_class, ast::sf::array_condition>;
	BOOST_SPIRIT_DECLARE(array_condition_type)

	using boolean_condition_type = x3::rule<boolean_condition_class, ast::sf::boolean_condition>;
	BOOST_SPIRIT_DECLARE(boolean_condition_type)

	using socket_group_condition_type = x3::rule<socket_group_condition_class, ast::sf::socket_group_condition>;
	BOOST_SPIRIT_DECLARE(socket_group_condition_type)

	using condition_type = x3::rule<condition_class, ast::sf::condition>;
	BOOST_SPIRIT_DECLARE(condition_type)

	using compound_action_type = x3::rule<compound_action_class, ast::sf::compound_action>;
	BOOST_SPIRIT_DECLARE(compound_action_type)

	using unary_action_type = x3::rule<unary_action_class, ast::sf::unary_action>;
	BOOST_SPIRIT_DECLARE(unary_action_type)

	using action_type = x3::rule<action_class, ast::sf::action>;
	BOOST_SPIRIT_DECLARE(action_type)

	// ---- filter structure ----

	using statement_type = x3::rule<statement_class, ast::sf::statement>;
	BOOST_SPIRIT_DECLARE(statement_type)

	using rule_block_type = x3::rule<rule_block_class, ast::sf::rule_block>;
	BOOST_SPIRIT_DECLARE(rule_block_type)

	using filter_structure_type = x3::rule<filter_structure_class, ast::sf::filter_structure>;
	BOOST_SPIRIT_DECLARE(filter_structure_type)

	using grammar_type = x3::rule<grammar_class, ast::sf::ast_type>;
	BOOST_SPIRIT_DECLARE(grammar_type)

} // namespace sf

namespace rf
{
	// ---- literal types ----

	using color_literal_type = x3::rule<color_literal_class, ast::rf::color_literal>;
	BOOST_SPIRIT_DECLARE(color_literal_type)

	using icon_literal_type = x3::rule<icon_literal_class, ast::rf::icon_literal>;
	BOOST_SPIRIT_DECLARE(icon_literal_type)

	using string_literal_array_type = x3::rule<string_literal_array_class, ast::rf::string_literal_array>;
	BOOST_SPIRIT_DECLARE(string_literal_array_type)

	using influence_literal_array_type = x3::rule<influence_literal_array_class, ast::rf::influence_literal_array>;
	BOOST_SPIRIT_DECLARE(influence_literal_array_type)

	// ---- conditions ----

	using rarity_condition_type               = x3::rule<rarity_condition_class,            ast::rf::rarity_condition>;
	using numeric_condition_type              = x3::rule<numeric_condition_class,           ast::rf::numeric_condition>;
	using string_array_condition_type         = x3::rule<string_array_condition_class,      ast::rf::string_array_condition>;
	using has_influence_condition_type        = x3::rule<has_influence_condition_class,     ast::rf::has_influence_condition>;
	using socket_group_condition_type         = x3::rule<socket_group_condition_class,      ast::rf::socket_group_condition>;
	using boolean_condition_type              = x3::rule<boolean_condition_class,           ast::rf::boolean_condition>;
	BOOST_SPIRIT_DECLARE(
		rarity_condition_type,
		numeric_condition_type,
		string_array_condition_type,
		has_influence_condition_type,
		socket_group_condition_type,
		boolean_condition_type
	)

	// ---- actions ----

	using color_action_type                          = x3::rule<color_action_class,                       ast::rf::color_action>;
	using set_font_size_action_type                  = x3::rule<set_font_size_action_class,               ast::rf::set_font_size_action>;
	using play_alert_sound_action_type               = x3::rule<play_alert_sound_action_class,            ast::rf::play_alert_sound_action>;
	using play_alert_sound_positional_action_type    = x3::rule<play_alert_sound_positional_action_class, ast::rf::play_alert_sound_positional_action>;
	using custom_alert_sound_action_type             = x3::rule<custom_alert_sound_action_class,          ast::rf::custom_alert_sound_action>;
	using disable_drop_sound_action_type             = x3::rule<disable_drop_sound_action_class,          ast::rf::disable_drop_sound_action>;
	using minimap_icon_action_type                   = x3::rule<minimap_icon_action_class,                ast::rf::minimap_icon_action>;
	using play_effect_action_type                    = x3::rule<play_effect_action_class,                 ast::rf::play_effect_action>;
	BOOST_SPIRIT_DECLARE(
		color_action_type,
		set_font_size_action_type,
		play_alert_sound_action_type,
		play_alert_sound_positional_action_type,
		custom_alert_sound_action_type,
		disable_drop_sound_action_type,
		minimap_icon_action_type,
		play_effect_action_type
	)

	// ---- filter structure ----

	using condition_type                      = x3::rule<condition_class,                   ast::rf::condition>;
	using action_type                         = x3::rule<action_class,                      ast::rf::action>;
	using rule_type                           = x3::rule<rule_class,                        ast::rf::rule>;
	using visibility_statement_type           = x3::rule<visibility_statement_class,        ast::rf::visibility_statement>;
	using filter_block_type                   = x3::rule<filter_block_class,                ast::rf::filter_block>;
	using grammar_type                        = x3::rule<grammar_class,                     ast::rf::ast_type>;
	BOOST_SPIRIT_DECLARE(
		condition_type,
		action_type,
		rule_type,
		visibility_statement_type,
		filter_block_type,
		grammar_type
	)
} // namespace rf

skipper_type sf_skipper();
skipper_type rf_skipper();
sf::grammar_type sf_grammar();
rf::grammar_type rf_grammar();

} // namespace fs::parser::detail
