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
	struct floating_point_literal_class          : error_on_error, annotate_on_success {};
	struct string_literal_class                  : error_on_error, annotate_on_success {};
	struct socket_spec_literal_class             : error_on_error, annotate_on_success {};
	struct boolean_literal_class                 : error_on_error, annotate_on_success {};
	struct rarity_literal_class                  : error_on_error, annotate_on_success {};
	struct shape_literal_class                   : error_on_error, annotate_on_success {};
	struct suit_literal_class                    : error_on_error, annotate_on_success {};
	struct influence_literal_class               : error_on_error, annotate_on_success {};
	struct shaper_voice_line_literal_class       : error_on_error, annotate_on_success {};
	struct gem_quality_type_literal_class        : error_on_error, annotate_on_success {};
	struct temp_literal_class                    : error_on_error, annotate_on_success {};
	struct none_literal_class                    : error_on_error, annotate_on_success {};

	struct literal_expression_class              : error_on_error, annotate_on_success {};
	struct comparison_operator_expression_class  : error_on_error, annotate_on_success {};
	struct exact_matching_policy_expression_class: error_on_error, annotate_on_success {};

	struct static_visibility_statement_class     : error_on_error, annotate_on_success {};
	struct continue_statement_class              : error_on_error, annotate_on_success {};
}

namespace sf
{
	// ---- whitespace ----

	// (nothing - whitespace does not synthesize any attributes)

	// ---- fundamental tokens ----

	struct name_class                            : error_on_error, annotate_on_success {};

	// ---- literal types ----

	// (all in common)

	// ---- expressions ----

	struct item_category_expression_class        : error_on_error, annotate_on_success {};
	struct primitive_value_class                 : error_on_error, annotate_on_success {};
	struct sequence_class                        : error_on_error, annotate_on_success {};
	struct statement_list_expression_class       : error_on_error, annotate_on_success {};
	struct value_expression_class                : error_on_error, annotate_on_success {};

	// ---- definitions ----

	struct constant_definition_class             : error_on_error, annotate_on_success {};
	struct definition_class                      : error_on_error, annotate_on_success {};

	// ---- conditions ----

	struct autogen_condition_class               : error_on_error, annotate_on_success {};
	struct price_comparison_condition_class      : error_on_error, annotate_on_success {};
	struct rarity_comparison_condition_class     : error_on_error, annotate_on_success {};
	struct numeric_comparison_condition_class    : error_on_error, annotate_on_success {};
	struct string_array_condition_class          : error_on_error, annotate_on_success {};
	struct ranged_string_array_condition_class   : error_on_error, annotate_on_success {};
	struct has_influence_condition_class         : error_on_error, annotate_on_success {};
	struct gem_quality_type_condition_class      : error_on_error, annotate_on_success {};
	struct socket_spec_condition_class           : error_on_error, annotate_on_success {};
	struct boolean_condition_class               : error_on_error, annotate_on_success {};
	struct condition_class                       : error_on_error, annotate_on_success {};

	// ---- actions ----

	struct set_color_action_class                : error_on_error, annotate_on_success {};
	struct set_font_size_action_class            : error_on_error, annotate_on_success {};
	struct minimap_icon_action_class             : error_on_error, annotate_on_success {};
	struct play_effect_action_class              : error_on_error, annotate_on_success {};
	struct play_alert_sound_action_class         : error_on_error, annotate_on_success {};
	struct custom_alert_sound_action_class       : error_on_error, annotate_on_success {};
	struct set_alert_sound_action_class          : error_on_error, annotate_on_success {};
	struct switch_drop_sound_action_class        : error_on_error, annotate_on_success {};
	struct action_class                          : error_on_error, annotate_on_success {};

	// ---- filter structure ----

	struct expand_statement_class                : error_on_error, annotate_on_success {};
	struct dynamic_visibility_policy_class       : error_on_error, annotate_on_success {};
	struct dynamic_visibility_statement_class    : error_on_error, annotate_on_success {};
	struct visibility_statement_class            : error_on_error, annotate_on_success {};
	struct behavior_statement_class              : error_on_error, annotate_on_success {};
	struct statement_class                       : error_on_error, annotate_on_success {};
	struct rule_block_class                      : error_on_error, annotate_on_success {};
	struct filter_structure_class                : error_on_error, annotate_on_success {};

	struct grammar_class                         : error_on_error, annotate_on_success {};
} // namespace sf

namespace rf
{
	// ---- literal types ----

	struct color_literal_class                   : error_on_error, annotate_on_success {};
	struct icon_literal_class                    : error_on_error, annotate_on_success {};
	struct string_literal_array_class            : error_on_error, annotate_on_success {};
	struct influence_literal_array_class         : error_on_error, annotate_on_success {};
	struct influence_spec_class                  : error_on_error, annotate_on_success {};

	// ---- expression ----

	struct literal_sequence_class                : error_on_error, annotate_on_success {};

	// ---- conditions ----

	struct rarity_condition_class                : error_on_error, annotate_on_success {};
	struct numeric_condition_class               : error_on_error, annotate_on_success {};
	struct string_array_condition_class          : error_on_error, annotate_on_success {};
	struct ranged_string_array_condition_class   : error_on_error, annotate_on_success {};
	struct has_influence_condition_class         : error_on_error, annotate_on_success {};
	struct gem_quality_type_condition_class      : error_on_error, annotate_on_success {};
	struct socket_spec_condition_class           : error_on_error, annotate_on_success {};
	struct boolean_condition_class               : error_on_error, annotate_on_success {};

	// ---- actions ----

	struct color_action_class                    : error_on_error, annotate_on_success {};
	struct set_font_size_action_class            : error_on_error, annotate_on_success {};
	struct play_alert_sound_action_class         : error_on_error, annotate_on_success {};
	struct custom_alert_sound_action_class       : error_on_error, annotate_on_success {};
	struct switch_drop_sound_action_class        : error_on_error, annotate_on_success {};
	struct minimap_icon_action_class             : error_on_error, annotate_on_success {};
	struct play_effect_action_class              : error_on_error, annotate_on_success {};

	// ---- filter structure ----

	struct condition_class                       : error_on_error, annotate_on_success {};
	struct action_class                          : error_on_error, annotate_on_success {};
	struct rule_class                            : error_on_error, annotate_on_success {};
	struct static_visibility_statement_class     : error_on_error, annotate_on_success {};
	struct filter_block_class                    : error_on_error, annotate_on_success {};
	struct grammar_class                         : error_on_error, annotate_on_success {};
} // namespace rf

namespace common
{
	// ---- whitespace ----

	// whitespace_type should be defined here but it has been moved to parser/config.hpp for
	// dependency reasons. See config.hpp for details.
	BOOST_SPIRIT_DECLARE(whitespace_type)

	using non_eol_whitespace_type = x3::rule<struct non_eol_whitespace_class /*, intentionally nothing */>;
	BOOST_SPIRIT_DECLARE(non_eol_whitespace_type)

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

	using floating_point_literal_type = x3::rule<floating_point_literal_class, ast::common::floating_point_literal>;
	BOOST_SPIRIT_DECLARE(floating_point_literal_type)

	using integer_literal_type = x3::rule<integer_literal_class, ast::common::integer_literal>;
	BOOST_SPIRIT_DECLARE(integer_literal_type)

	using string_literal_type = x3::rule<string_literal_class, ast::common::string_literal>;
	BOOST_SPIRIT_DECLARE(string_literal_type)

	using socket_spec_literal_type = x3::rule<socket_spec_literal_class, ast::common::socket_spec_literal>;
	BOOST_SPIRIT_DECLARE(socket_spec_literal_type)

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

	using shaper_voice_line_literal_type = x3::rule<shaper_voice_line_literal_class, ast::common::shaper_voice_line_literal>;
	BOOST_SPIRIT_DECLARE(shaper_voice_line_literal_type)

	using gem_quality_type_literal_type = x3::rule<gem_quality_type_literal_class, ast::common::gem_quality_type_literal>;
	BOOST_SPIRIT_DECLARE(gem_quality_type_literal_type)

	using temp_literal_type = x3::rule<temp_literal_class, ast::common::temp_literal>;
	BOOST_SPIRIT_DECLARE(temp_literal_type)

	using none_literal_type = x3::rule<none_literal_class, ast::common::none_literal>;
	BOOST_SPIRIT_DECLARE(none_literal_type)

	// ---- expressions ----

	using literal_expression_type = x3::rule<literal_expression_class, ast::common::literal_expression>;
	BOOST_SPIRIT_DECLARE(literal_expression_type)

	using comparison_operator_expression_type = x3::rule<comparison_operator_expression_class, ast::common::comparison_operator_expression>;
	BOOST_SPIRIT_DECLARE(comparison_operator_expression_type)

	using exact_matching_policy_expression_type = x3::rule<exact_matching_policy_expression_class, ast::common::exact_matching_policy>;
	BOOST_SPIRIT_DECLARE(exact_matching_policy_expression_type)

	// ---- filter structure ----

	using static_visibility_statement_type = x3::rule<static_visibility_statement_class, ast::common::static_visibility_statement>;
	BOOST_SPIRIT_DECLARE(static_visibility_statement_type)

	using continue_statement_type = x3::rule<continue_statement_class, ast::common::continue_statement>;
	BOOST_SPIRIT_DECLARE(continue_statement_type)
} // namespace common

namespace sf
{
	// ---- whitespace ----

	// (all in common)

	// ---- fundamental tokens ----

	using name_type = x3::rule<name_class, ast::sf::name>;
	BOOST_SPIRIT_DECLARE(name_type)

	// ---- literal types ----

	// (all in common)

	// ---- expressions ----

	using item_category_expression_type = x3::rule<item_category_expression_class, ast::sf::item_category_expression>;
	BOOST_SPIRIT_DECLARE(item_category_expression_type)

	using primitive_value_type = x3::rule<primitive_value_class, ast::sf::primitive_value>;
	BOOST_SPIRIT_DECLARE(primitive_value_type)

	using sequence_type = x3::rule<sequence_class, ast::sf::sequence>;
	BOOST_SPIRIT_DECLARE(sequence_type)

	using statement_list_expression_type = x3::rule<statement_list_expression_class, ast::sf::statement_list_expression>;
	BOOST_SPIRIT_DECLARE(statement_list_expression_type)

	using value_expression_type = x3::rule<value_expression_class, ast::sf::value_expression>;
	BOOST_SPIRIT_DECLARE(value_expression_type)

	// ---- definitions ----

	using constant_definition_type = x3::rule<constant_definition_class, ast::sf::constant_definition>;
	BOOST_SPIRIT_DECLARE(constant_definition_type)

	using definition_type = x3::rule<definition_class, ast::sf::definition>;
	BOOST_SPIRIT_DECLARE(definition_type)

	// ---- conditions ----

	using autogen_condition_type = x3::rule<autogen_condition_class, ast::sf::autogen_condition>;
	BOOST_SPIRIT_DECLARE(autogen_condition_type)

	using price_comparison_condition_type = x3::rule<price_comparison_condition_class, ast::sf::price_comparison_condition>;
	BOOST_SPIRIT_DECLARE(price_comparison_condition_type)

	using rarity_comparison_condition_type = x3::rule<rarity_comparison_condition_class, ast::sf::rarity_comparison_condition>;
	BOOST_SPIRIT_DECLARE(rarity_comparison_condition_type)

	using numeric_comparison_condition_type = x3::rule<numeric_comparison_condition_class, ast::sf::numeric_comparison_condition>;
	BOOST_SPIRIT_DECLARE(numeric_comparison_condition_type)

	using string_array_condition_type = x3::rule<string_array_condition_class, ast::sf::string_array_condition>;
	BOOST_SPIRIT_DECLARE(string_array_condition_type)

	using ranged_string_array_condition_type = x3::rule<ranged_string_array_condition_class, ast::sf::ranged_string_array_condition>;
	BOOST_SPIRIT_DECLARE(ranged_string_array_condition_type)

	using has_influence_condition_type = x3::rule<has_influence_condition_class, ast::sf::has_influence_condition>;
	BOOST_SPIRIT_DECLARE(has_influence_condition_type)

	using gem_quality_type_condition_type = x3::rule<gem_quality_type_condition_class, ast::sf::gem_quality_type_condition>;
	BOOST_SPIRIT_DECLARE(gem_quality_type_condition_type)

	using socket_spec_condition_type = x3::rule<socket_spec_condition_class, ast::sf::socket_spec_condition>;
	BOOST_SPIRIT_DECLARE(socket_spec_condition_type)

	using boolean_condition_type = x3::rule<boolean_condition_class, ast::sf::boolean_condition>;
	BOOST_SPIRIT_DECLARE(boolean_condition_type)

	using condition_type = x3::rule<condition_class, ast::sf::condition>;
	BOOST_SPIRIT_DECLARE(condition_type)

	// ---- actions ----

	using set_color_action_type = x3::rule<set_color_action_class, ast::sf::set_color_action>;
	BOOST_SPIRIT_DECLARE(set_color_action_type)

	using set_font_size_action_type = x3::rule<set_font_size_action_class, ast::sf::set_font_size_action>;
	BOOST_SPIRIT_DECLARE(set_font_size_action_type)

	using minimap_icon_action_type = x3::rule<minimap_icon_action_class, ast::sf::minimap_icon_action>;
	BOOST_SPIRIT_DECLARE(minimap_icon_action_type)

	using play_effect_action_type = x3::rule<play_effect_action_class, ast::sf::play_effect_action>;
	BOOST_SPIRIT_DECLARE(play_effect_action_type)

	using play_alert_sound_action_type = x3::rule<play_alert_sound_action_class, ast::sf::play_alert_sound_action>;
	BOOST_SPIRIT_DECLARE(play_alert_sound_action_type)

	using custom_alert_sound_action_type = x3::rule<custom_alert_sound_action_class, ast::sf::custom_alert_sound_action>;
	BOOST_SPIRIT_DECLARE(custom_alert_sound_action_type)

	using set_alert_sound_action_type = x3::rule<set_alert_sound_action_class, ast::sf::set_alert_sound_action>;
	BOOST_SPIRIT_DECLARE(set_alert_sound_action_type)

	using switch_drop_sound_action_type = x3::rule<switch_drop_sound_action_class, ast::sf::switch_drop_sound_action>;
	BOOST_SPIRIT_DECLARE(switch_drop_sound_action_type)

	using action_type = x3::rule<action_class, ast::sf::action>;
	BOOST_SPIRIT_DECLARE(action_type)

	// ---- filter structure ----

	using expand_statement_type = x3::rule<expand_statement_class, ast::sf::expand_statement>;
	BOOST_SPIRIT_DECLARE(expand_statement_type)

	using dynamic_visibility_policy_type = x3::rule<dynamic_visibility_policy_class, ast::sf::dynamic_visibility_policy>;
	BOOST_SPIRIT_DECLARE(dynamic_visibility_policy_type)

	using dynamic_visibility_statement_type = x3::rule<dynamic_visibility_statement_class, ast::sf::dynamic_visibility_statement>;
	BOOST_SPIRIT_DECLARE(dynamic_visibility_statement_type)

	using visibility_statement_type = x3::rule<visibility_statement_class, ast::sf::visibility_statement>;
	BOOST_SPIRIT_DECLARE(visibility_statement_type)

	using behavior_statement_type = x3::rule<behavior_statement_class, ast::sf::behavior_statement>;
	BOOST_SPIRIT_DECLARE(behavior_statement_type)

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

	using string_literal_array_type = x3::rule<string_literal_array_class, ast::rf::string_literal_array>;
	BOOST_SPIRIT_DECLARE(string_literal_array_type)

	using influence_literal_array_type = x3::rule<influence_literal_array_class, ast::rf::influence_literal_array>;
	BOOST_SPIRIT_DECLARE(influence_literal_array_type)

	using influence_spec_type = x3::rule<influence_spec_class, ast::rf::influence_spec>;
	BOOST_SPIRIT_DECLARE(influence_spec_type)

	// ---- expressions ----

	using literal_sequence_type               = x3::rule<literal_sequence_class,            ast::rf::literal_sequence>;

	// ---- conditions ----

	using rarity_condition_type               = x3::rule<rarity_condition_class,              ast::rf::rarity_condition>;
	using numeric_condition_type              = x3::rule<numeric_condition_class,             ast::rf::numeric_condition>;
	using string_array_condition_type         = x3::rule<string_array_condition_class,        ast::rf::string_array_condition>;
	using ranged_string_array_condition_type  = x3::rule<ranged_string_array_condition_class, ast::rf::ranged_string_array_condition>;
	using has_influence_condition_type        = x3::rule<has_influence_condition_class,       ast::rf::has_influence_condition>;
	using gem_quality_type_condition_type     = x3::rule<gem_quality_type_condition_class,    ast::rf::gem_quality_type_condition>;
	using socket_spec_condition_type          = x3::rule<socket_spec_condition_class,         ast::rf::socket_spec_condition>;
	using boolean_condition_type              = x3::rule<boolean_condition_class,             ast::rf::boolean_condition>;
	BOOST_SPIRIT_DECLARE(
		rarity_condition_type,
		numeric_condition_type,
		string_array_condition_type,
		ranged_string_array_condition_type,
		has_influence_condition_type,
		gem_quality_type_condition_type,
		socket_spec_condition_type,
		boolean_condition_type
	)

	// ---- actions ----

	using color_action_type                          = x3::rule<color_action_class,                       ast::rf::color_action>;
	using set_font_size_action_type                  = x3::rule<set_font_size_action_class,               ast::rf::set_font_size_action>;
	using play_alert_sound_action_type               = x3::rule<play_alert_sound_action_class,            ast::rf::play_alert_sound_action>;
	using custom_alert_sound_action_type             = x3::rule<custom_alert_sound_action_class,          ast::rf::custom_alert_sound_action>;
	using switch_drop_sound_action_type              = x3::rule<switch_drop_sound_action_class,           ast::rf::switch_drop_sound_action>;
	using minimap_icon_action_type                   = x3::rule<minimap_icon_action_class,                ast::rf::minimap_icon_action>;
	using play_effect_action_type                    = x3::rule<play_effect_action_class,                 ast::rf::play_effect_action>;
	BOOST_SPIRIT_DECLARE(
		color_action_type,
		set_font_size_action_type,
		play_alert_sound_action_type,
		custom_alert_sound_action_type,
		switch_drop_sound_action_type,
		minimap_icon_action_type,
		play_effect_action_type
	)

	// ---- filter structure ----

	using condition_type                      = x3::rule<condition_class,                   ast::rf::condition>;
	using action_type                         = x3::rule<action_class,                      ast::rf::action>;
	using rule_type                           = x3::rule<rule_class,                        ast::rf::rule>;
	using static_visibility_statement_type    = x3::rule<static_visibility_statement_class, ast::rf::static_visibility_statement>;
	using filter_block_type                   = x3::rule<filter_block_class,                ast::rf::filter_block>;
	using grammar_type                        = x3::rule<grammar_class,                     ast::rf::ast_type>;
	BOOST_SPIRIT_DECLARE(
		condition_type,
		action_type,
		rule_type,
		static_visibility_statement_type,
		filter_block_type,
		grammar_type
	)
} // namespace rf

skipper_type sf_skipper();
skipper_type rf_skipper();
sf::grammar_type sf_grammar();
rf::grammar_type rf_grammar();

} // namespace fs::parser::detail
