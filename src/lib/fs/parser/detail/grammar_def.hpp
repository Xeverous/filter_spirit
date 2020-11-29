/**
 * @file grammar type definitions
 *
 * @details This file is intended for core grammar definitions.
 * Use BOOST_SPIRIT_DEFINE here.
 */
#pragma once
#include <fs/parser/detail/symbols.hpp>
#include <fs/parser/detail/grammar.hpp>

namespace fs::parser::detail
{

// Spirit coding style:
// - rule ID        : foo_class
// - rule type      : foo_type
// - rule definition: foo_def
// - rule object    : foo

namespace common
{
	// ---- whitespace ----

	const comment_type comment = "comment";
	const auto comment_def = x3::lit('#') > *(x3::char_ - x3::eol) > (x3::eol | x3::eoi);
	BOOST_SPIRIT_DEFINE(comment)

	const whitespace_type whitespace = "whitespace";
	const auto whitespace_def = x3::space | comment;
	BOOST_SPIRIT_DEFINE(whitespace)

	const non_eol_whitespace_type non_eol_whitespace = "non-eol whitespace";
	const auto non_eol_whitespace_def = x3::space - x3::eol;
	BOOST_SPIRIT_DEFINE(non_eol_whitespace)

	// ---- fundamental tokens ----

	const identifier_impl_type identifier_impl = "identifier";
	const auto identifier_impl_def = x3::lexeme[(x3::alpha | x3::char_('_')) > *(x3::alnum | x3::char_('_'))];
	BOOST_SPIRIT_DEFINE(identifier_impl)

	const identifier_type identifier = identifier_impl.name;
	const auto identifier_def = identifier_impl;
	BOOST_SPIRIT_DEFINE(identifier)

	const not_alnum_or_underscore_type not_alnum_or_underscore = "not alphanumeric character or underscore";
	const auto not_alnum_or_underscore_def = !&(x3::alnum | '_');
	BOOST_SPIRIT_DEFINE(not_alnum_or_underscore)

	/*
	 * Using just x3::lit("str") for keywords is not enough, because X3 implementes LL parser.
	 * It does not perform any lookahead by default, so "PlayAlertSoundPositional" is already
	 * successfully matched at the point of reading "PlayAlertSound" and variable names like
	 * "Truee" stop on matching keyword "True". Add a simple lookahead to make sure there are
	 * no more alphanumeric characters or '_' just after the literal. Wrap in lexeme to inhibit
	 * running skip parser - keywords always need to be written without whitespace between letters.
	 */
	template <typename T>
	auto make_keyword(T grammar)
	{
		return x3::lexeme[grammar >> not_alnum_or_underscore];
	}

	// ---- literal types ----

	const integer_literal_type integer_literal = "number (integer)";
	const auto integer_literal_def = x3::int_;
	BOOST_SPIRIT_DEFINE(integer_literal)

	const string_literal_type string_literal = "string";
	const auto string_literal_def = x3::lexeme['"' > *(x3::char_ - (x3::lit('"') | '\n' | '\r')) > '"'];
	BOOST_SPIRIT_DEFINE(string_literal)

	const boolean_literal_type boolean_literal = "boolean literal";
	const auto boolean_literal_def = make_keyword(symbols::rf::booleans);
	BOOST_SPIRIT_DEFINE(boolean_literal)

	const rarity_literal_type rarity_literal = "rarity literal";
	const auto rarity_literal_def = make_keyword(symbols::rf::rarities);
	BOOST_SPIRIT_DEFINE(rarity_literal)

	const shape_literal_type shape_literal = "shape literal";
	const auto shape_literal_def = make_keyword(symbols::rf::shapes);
	BOOST_SPIRIT_DEFINE(shape_literal)

	const suit_literal_type suit_literal = "suit literal";
	const auto suit_literal_def = make_keyword(symbols::rf::suits);
	BOOST_SPIRIT_DEFINE(suit_literal)

	const influence_literal_type influence_literal = "influence literal";
	const auto influence_literal_def = make_keyword(symbols::rf::influences);
	BOOST_SPIRIT_DEFINE(influence_literal)

	const shaper_voice_line_literal_type shaper_voice_line_literal = "shaper voice line literal";
	const auto shaper_voice_line_literal_def = make_keyword(symbols::rf::shaper_voice_lines);
	BOOST_SPIRIT_DEFINE(shaper_voice_line_literal)

	const gem_quality_type_literal_type gem_quality_type_literal = "gem quality type literal";
	const auto gem_quality_type_literal_def = make_keyword(symbols::rf::gem_quality_types);
	BOOST_SPIRIT_DEFINE(gem_quality_type_literal)

	const temp_literal_type temp_literal = "temp literal";
	const auto temp_literal_def = make_keyword(lang::keywords::rf::temp) > x3::attr(ast::common::temp_literal{});
	BOOST_SPIRIT_DEFINE(temp_literal)

	const none_literal_type none_literal = "none literal";
	const auto none_literal_def = make_keyword(lang::keywords::rf::none) > x3::attr(ast::common::none_literal{});
	BOOST_SPIRIT_DEFINE(none_literal)

	// ---- rules ----

	const comparison_operator_expression_type comparison_operator_expression = "comparison operator";
	const auto comparison_operator_expression_def = symbols::rf::comparison_operators | x3::attr(lang::comparison_type::equal_soft);
	BOOST_SPIRIT_DEFINE(comparison_operator_expression)

	const exact_matching_policy_expression_type exact_matching_policy_expression = "exact matching policy expression";
	const auto exact_matching_policy_expression_def = ("==" > x3::attr(true)) | ("=" > x3::attr(false)) | x3::attr(false);
	BOOST_SPIRIT_DEFINE(exact_matching_policy_expression)

	const visibility_statement_type visibility_statement = "visibility statement";
	const auto visibility_statement_def = make_keyword(symbols::rf::visibility_literals);
	BOOST_SPIRIT_DEFINE(visibility_statement)

	const continue_statement_type continue_statement = "continue statement";
	const auto continue_statement_def = make_keyword(lang::keywords::rf::continue_);
	BOOST_SPIRIT_DEFINE(continue_statement)
} // namespace common

namespace sf
{
	using common::make_keyword;

	// ---- fundamental tokens ----

	const name_type name = "name";
	const auto name_def = x3::lexeme['$' >> common::identifier];
	BOOST_SPIRIT_DEFINE(name)

	// ---- literal types ----

	/*
	 * Enable strict parsing policy for floating-point expressions. This will cause to fail the match
	 * if the expression does not contain '.' or exponent. This policy is necessary to avoid parsing
	 * integer literals as valid floating-point literals.
	 */
	const floating_point_literal_type floating_point_literal = "number (fractional)";
	const auto floating_point_literal_def = x3::real_parser<double, x3::strict_real_policies<double>>{};
	BOOST_SPIRIT_DEFINE(floating_point_literal)

	const socket_spec_literal_type socket_spec_literal = "socket spec literal";
	const auto socket_spec_literal_def = x3::lexeme[-common::integer_literal >> common::identifier];
	BOOST_SPIRIT_DEFINE(socket_spec_literal)

	// ---- expressions ----

	// moved here due to circular dependency
	const action_type action = "action";

	const compound_action_expression_type compound_action_expression = "compound action expression";
	const auto compound_action_expression_def = '{' >> *action > '}';
	BOOST_SPIRIT_DEFINE(compound_action_expression)

	const literal_expression_type literal_expression = "literal";
	const auto literal_expression_def =
	// 1. Order here is crucial in context-sensitive grammars
	// 2. Order should match types in literal_expression_type::attribute_type
		// Keyword literals first. They are the most unique (only 1 allowed pattern of characters)
		  common::boolean_literal
		| common::rarity_literal
		| common::shape_literal
		| common::suit_literal
		| common::influence_literal
		| common::shaper_voice_line_literal
		| common::gem_quality_type_literal
		| common::temp_literal
		| common::none_literal
		// 1. Must be attempted before integer literal, otherwise integer literal for eg "5GGG" would
		// leave letters which immediately follow (no whitespace) that would trigger parse error
		// 2. Must be attempted after all keyword literals, otherwise they will be incorrectly
		// attempted as a socket spec and error as they do not consist only of R/G/B/W/A/D letters
		| socket_spec_literal
		// Must be attempted before integer literal. Otherwise integer literal will just consume
		// the number and do not check if there is any . character following it.
		| floating_point_literal
		| common::integer_literal
		| common::string_literal;
	BOOST_SPIRIT_DEFINE(literal_expression)

	const item_category_expression_type item_category_expression = "item category expression";
	const auto item_category_expression_def = make_keyword(symbols::sf::item_categories);
	BOOST_SPIRIT_DEFINE(item_category_expression)

	const primitive_value_type primitive_value = "primitive";
	const auto primitive_value_def = name | literal_expression;
	BOOST_SPIRIT_DEFINE(primitive_value)

	const sequence_type sequence = "sequence";
	const auto sequence_def = x3::skip(common::non_eol_whitespace)[+primitive_value];
	BOOST_SPIRIT_DEFINE(sequence)

	const value_expression_type value_expression = "expression";
	const auto value_expression_def = compound_action_expression | sequence;
	BOOST_SPIRIT_DEFINE(value_expression)

	// ---- definitions ----

	const constant_definition_type constant_definition = "constant definiton";
	const auto constant_definition_def = name >> '=' > value_expression;
	BOOST_SPIRIT_DEFINE(constant_definition)

	// future space for metedata definitions
	const definition_type definition = "definition";
	const auto definition_def = constant_definition;
	BOOST_SPIRIT_DEFINE(definition)

	// ---- conditions ----

	const autogen_condition_type autogen_condition = "autogen condition";
	const auto autogen_condition_def = make_keyword(lang::keywords::sf::autogen) > item_category_expression;
	BOOST_SPIRIT_DEFINE(autogen_condition)

	const price_comparison_condition_type price_comparison_condition = "price comparison condition";
	const auto price_comparison_condition_def =
		make_keyword(lang::keywords::sf::price)
		> common::comparison_operator_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(price_comparison_condition)

	const rarity_comparison_condition_type rarity_comparison_condition = "rarity comparison condition";
	const auto rarity_comparison_condition_def =
		make_keyword(lang::keywords::rf::rarity)
		> common::comparison_operator_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(rarity_comparison_condition)

	const numeric_comparison_condition_type numeric_comparison_condition = "numeric comparison condition";
	const auto numeric_comparison_condition_def =
		make_keyword(symbols::rf::numeric_comparison_condition_properties)
		> common::comparison_operator_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(numeric_comparison_condition)

	const string_array_condition_type string_array_condition = "string array condition";
	const auto string_array_condition_def =
		make_keyword(symbols::rf::string_array_condition_properties)
		> common::exact_matching_policy_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(string_array_condition)

	const has_influence_condition_type has_influence_condition = "has influence condition";
	const auto has_influence_condition_def =
		make_keyword(lang::keywords::rf::has_influence)
		> common::exact_matching_policy_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(has_influence_condition)

	const gem_quality_type_condition_type gem_quality_type_condition = "gem quality type condition";
	const auto gem_quality_type_condition_def = make_keyword(lang::keywords::rf::gem_quality_type) > sequence;
	BOOST_SPIRIT_DEFINE(gem_quality_type_condition)

	const socket_spec_condition_type socket_spec_condition = "socket spec condition";
	const auto socket_spec_condition_def =
		make_keyword(symbols::rf::socket_spec_condition_properties)
		> common::comparison_operator_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(socket_spec_condition)

	const boolean_condition_type boolean_condition = "boolean condition";
	const auto boolean_condition_def =
		make_keyword(symbols::rf::boolean_condition_properties)
		> sequence;
	BOOST_SPIRIT_DEFINE(boolean_condition)

	const condition_type condition = "condition";
	const auto condition_def =
		  autogen_condition
		| price_comparison_condition
		| rarity_comparison_condition
		| numeric_comparison_condition
		| string_array_condition
		| has_influence_condition
		| gem_quality_type_condition
		| socket_spec_condition
		| boolean_condition;
	BOOST_SPIRIT_DEFINE(condition)

	// ---- actions ----

	const set_color_action_type set_color_action = "set color action";
	const auto set_color_action_def = make_keyword(symbols::rf::color_actions) > sequence;
	BOOST_SPIRIT_DEFINE(set_color_action)

	const set_font_size_action_type set_font_size_action = "set font size action";
	const auto set_font_size_action_def = make_keyword(lang::keywords::rf::set_font_size) > sequence;
	BOOST_SPIRIT_DEFINE(set_font_size_action)

	const minimap_icon_action_type minimap_icon_action = "minimap icon action";
	const auto minimap_icon_action_def = make_keyword(lang::keywords::rf::minimap_icon) > sequence;
	BOOST_SPIRIT_DEFINE(minimap_icon_action)

	const play_effect_action_type play_effect_action = "play effect action";
	const auto play_effect_action_def = make_keyword(lang::keywords::rf::play_effect) > sequence;
	BOOST_SPIRIT_DEFINE(play_effect_action)

	const play_alert_sound_action_type play_alert_sound_action = "play alert sound action";
	const auto play_alert_sound_action_def = make_keyword(symbols::rf::play_alert_sound_actions) > sequence;
	BOOST_SPIRIT_DEFINE(play_alert_sound_action)

	const custom_alert_sound_action_type custom_alert_sound_action = "custom alert sound action";
	const auto custom_alert_sound_action_def = make_keyword(lang::keywords::rf::custom_alert_sound) > sequence;
	BOOST_SPIRIT_DEFINE(custom_alert_sound_action)

	const set_alert_sound_action_type set_alert_sound_action = "set alert sound action";
	const auto set_alert_sound_action_def = make_keyword(lang::keywords::sf::set_alert_sound) > sequence;
	BOOST_SPIRIT_DEFINE(set_alert_sound_action)

	const switch_drop_sound_action_type switch_drop_sound_action = "switch drop sound action";
	const auto switch_drop_sound_action_def =
		(make_keyword(lang::keywords::rf::enable_drop_sound)  > x3::attr(true)) |
		(make_keyword(lang::keywords::rf::disable_drop_sound) > x3::attr(false));
	BOOST_SPIRIT_DEFINE(switch_drop_sound_action)

	const compound_action_type compound_action = "compound action";
	const auto compound_action_def = make_keyword(lang::keywords::sf::set) > sequence;
	BOOST_SPIRIT_DEFINE(compound_action)

	const auto action_def =
		  set_color_action
		| set_font_size_action
		| minimap_icon_action
		| play_effect_action
		| play_alert_sound_action
		| custom_alert_sound_action
		| set_alert_sound_action
		| switch_drop_sound_action
		| compound_action;
	BOOST_SPIRIT_DEFINE(action)

	// ---- filter structure ----

	const behavior_statement_type behavior_statement = "behavior statement";
	const auto behavior_statement_def = common::visibility_statement > -common::continue_statement;
	BOOST_SPIRIT_DEFINE(behavior_statement)

	// moved here due to circular dependency
	const rule_block_type rule_block = "rule block";

	const statement_type statement = "statement";
	const auto statement_def = action | behavior_statement | rule_block;
	BOOST_SPIRIT_DEFINE(statement)

	const auto rule_block_def = *condition >> x3::lit('{') > *statement > x3::lit('}');
	BOOST_SPIRIT_DEFINE(rule_block)

	const filter_structure_type filter_structure = "filter structure";
	const auto filter_structure_def = *definition > *statement;
	BOOST_SPIRIT_DEFINE(filter_structure)

	const grammar_type grammar = "code";
	const auto grammar_def = filter_structure > x3::eoi;
	BOOST_SPIRIT_DEFINE(grammar)

} // namespace sf

namespace rf
{
	using common::make_keyword;

	// ---- literal types ----

	const color_literal_type color_literal = "color literal";
	const auto color_literal_def =
		  common::integer_literal
		> common::integer_literal
		> common::integer_literal
		> -common::integer_literal;
	BOOST_SPIRIT_DEFINE(color_literal)

	const icon_literal_type icon_literal = "icon literal";
	const auto icon_literal_def = common::integer_literal > make_keyword(common::suit_literal) > make_keyword(common::shape_literal);
	BOOST_SPIRIT_DEFINE(icon_literal)

	const string_literal_array_type string_literal_array = "1 or more string literals";
	const auto string_literal_array_def = +common::string_literal;
	BOOST_SPIRIT_DEFINE(string_literal_array)

	const influence_literal_array_type influence_literal_array = "1 or more influence literals";
	const auto influence_literal_array_def = +common::influence_literal;
	BOOST_SPIRIT_DEFINE(influence_literal_array)

	const influence_spec_type influence_spec = "influence spec";
	const auto influence_spec_def = common::none_literal | influence_literal_array;
	BOOST_SPIRIT_DEFINE(influence_spec)

	const socket_spec_literal_type socket_spec_literal = "socket spec literal";
	const auto socket_spec_literal_def =
		x3::lexeme[
			// TODO this value-initializes position tags - implement more advanced source location tracking
			// that will allow custom synthesised/virtual source positions
			(common::integer_literal >> (common::identifier | x3::attr(ast::rf::identifier{})))
			| (x3::attr(ast::rf::integer_literal{}) >> common::identifier)
		];
	BOOST_SPIRIT_DEFINE(socket_spec_literal)

	// ---- conditions ----

	const rarity_condition_type rarity_condition = "rarity condition";
	const auto rarity_condition_def = make_keyword(lang::keywords::rf::rarity) > common::comparison_operator_expression > common::rarity_literal;
	BOOST_SPIRIT_DEFINE(rarity_condition)

	const numeric_condition_type numeric_condition = "numeric condition";
	const auto numeric_condition_def =
		make_keyword(symbols::rf::numeric_comparison_condition_properties) > common::comparison_operator_expression > common::integer_literal;
	BOOST_SPIRIT_DEFINE(numeric_condition)

	const string_array_condition_type string_array_condition = "string array condition";
	const auto string_array_condition_def =
		make_keyword(symbols::rf::string_array_condition_properties) > common::exact_matching_policy_expression > string_literal_array;
	BOOST_SPIRIT_DEFINE(string_array_condition)

	const has_influence_condition_type has_influence_condition = "has influence condition";
	const auto has_influence_condition_def =
		make_keyword(lang::keywords::rf::has_influence) > common::exact_matching_policy_expression > influence_spec;
	BOOST_SPIRIT_DEFINE(has_influence_condition)

	const gem_quality_type_condition_type gem_quality_type_condition = "gem quality type condition";
	const auto gem_quality_type_condition_def = make_keyword(lang::keywords::rf::gem_quality_type) > common::gem_quality_type_literal;
	BOOST_SPIRIT_DEFINE(gem_quality_type_condition)

	const socket_spec_condition_type socket_spec_condition = "socket spec condition";
	const auto socket_spec_condition_def =
		make_keyword(symbols::rf::socket_spec_condition_properties) > common::comparison_operator_expression > +socket_spec_literal;
	BOOST_SPIRIT_DEFINE(socket_spec_condition)

	const boolean_condition_type boolean_condition = "boolean condition";
	const auto boolean_condition_def = make_keyword(symbols::rf::boolean_condition_properties) > common::boolean_literal;
	BOOST_SPIRIT_DEFINE(boolean_condition)

	const condition_type condition = "condition";
	const auto condition_def =
		rarity_condition
		| numeric_condition
		| string_array_condition
		| has_influence_condition
		| gem_quality_type_condition
		| socket_spec_condition
		| boolean_condition;
	BOOST_SPIRIT_DEFINE(condition)

	// ---- actions ----

	const color_action_type color_action = "color action";
	const auto color_action_def = make_keyword(symbols::rf::color_actions) > color_literal;
	BOOST_SPIRIT_DEFINE(color_action)

	const set_font_size_action_type set_font_size_action = "font size action";
	const auto set_font_size_action_def = make_keyword(lang::keywords::rf::set_font_size) > common::integer_literal;
	BOOST_SPIRIT_DEFINE(set_font_size_action)

	const sound_id_type sound_id = "sound ID";
	const auto sound_id_def = common::integer_literal | common::shaper_voice_line_literal;
	BOOST_SPIRIT_DEFINE(sound_id)

	const play_alert_sound_action_type play_alert_sound_action = "alert sound action";
	const auto play_alert_sound_action_def =
		make_keyword(lang::keywords::rf::play_alert_sound)
		> sound_id
		> -common::integer_literal; // volume (optional token)
	BOOST_SPIRIT_DEFINE(play_alert_sound_action)

	const play_alert_sound_positional_action_type play_alert_sound_positional_action = "positional alert sound action";
	const auto play_alert_sound_positional_action_def =
		make_keyword(lang::keywords::rf::play_alert_sound_positional)
		> sound_id
		> -common::integer_literal; // volume (optional token)
	BOOST_SPIRIT_DEFINE(play_alert_sound_positional_action)

	const custom_alert_sound_action_type custom_alert_sound_action = "custom alert sound action";
	const auto custom_alert_sound_action_def = make_keyword(lang::keywords::rf::custom_alert_sound) > common::string_literal;
	BOOST_SPIRIT_DEFINE(custom_alert_sound_action)

	const switch_drop_sound_action_type switch_drop_sound_action = "switch drop sound action";
	const auto switch_drop_sound_action_def =
		(make_keyword(lang::keywords::rf::enable_drop_sound)  > x3::attr(true)) |
		(make_keyword(lang::keywords::rf::disable_drop_sound) > x3::attr(false));
	BOOST_SPIRIT_DEFINE(switch_drop_sound_action)

	const minimap_icon_action_type minimap_icon_action = "minimap icon action";
	const auto minimap_icon_action_def = make_keyword(lang::keywords::rf::minimap_icon) > icon_literal;
	BOOST_SPIRIT_DEFINE(minimap_icon_action)

	const play_effect_action_type play_effect_action = "play effct action";
	const auto play_effect_action_def =
		make_keyword(lang::keywords::rf::play_effect)
		> common::suit_literal
		> ((make_keyword(lang::keywords::rf::temp) > x3::attr(true)) | x3::attr(false));
	BOOST_SPIRIT_DEFINE(play_effect_action)

	const action_type action = "action";
	const auto action_def =
		color_action
		| set_font_size_action
		| play_alert_sound_action
		| play_alert_sound_positional_action
		| custom_alert_sound_action
		| switch_drop_sound_action
		| minimap_icon_action
		| play_effect_action;
	BOOST_SPIRIT_DEFINE(action)

	// ---- filter structure ----

	const rule_type rule = "rule";
	const auto rule_def = condition | action;
	BOOST_SPIRIT_DEFINE(rule)

	const filter_block_type filter_block = "filter block";
	const auto filter_block_def = common::visibility_statement > *rule;
	BOOST_SPIRIT_DEFINE(filter_block)

	const grammar_type grammar = "code";
	const auto grammar_def = *filter_block;
	BOOST_SPIRIT_DEFINE(grammar)
} // namespace rf

} // namespace fs::parser::detail
