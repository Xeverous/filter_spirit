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
	const auto comment_def = x3::lit('#') > *(x3::char_ - x3::eol) > (x3::eol | &x3::eoi);
	BOOST_SPIRIT_DEFINE(comment)

	const whitespace_type whitespace = "whitespace";
	const auto whitespace_def = x3::space | comment;
	BOOST_SPIRIT_DEFINE(whitespace)

	const non_eol_whitespace_type non_eol_whitespace = "non-eol whitespace";
	const auto non_eol_whitespace_def = x3::space - x3::eol;
	BOOST_SPIRIT_DEFINE(non_eol_whitespace)

	const end_of_line_type end_of_line = "end of line or file";
	const auto end_of_line_def = comment | x3::eol | &x3::eoi;
	BOOST_SPIRIT_DEFINE(end_of_line)

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

	/*
	 * All conditions and most actions require a sequence of tokens.
	 * Disable eol in skip parser and require eol/eoi to end the sequence
	 * (for empty sequences it is the only way to know that they have 0 elements).
	 * This disallows code like "SocketGroup RGB {" but "{" can not be consistently
	 * put on the same line as many conditions will be very long (especially BaseType).
	 * TODO the skipper should be moved upwards in the grammar and applied to every command.
	 */
	template <typename T>
	auto make_sequence(T grammar)
	{
		return x3::skip(non_eol_whitespace)[*grammar > end_of_line];
	}

	// ---- literal types ----

	/*
	 * Enable strict parsing policy for floating-point expressions. This will cause to fail the match
	 * if the expression does not contain '.' or exponent. This policy is necessary to avoid parsing
	 * integer literals as valid floating-point literals.
	 */
	const floating_point_literal_type floating_point_literal = "number (fractional)";
	const auto floating_point_literal_def = x3::real_parser<double, x3::strict_real_policies<double>>{};
	BOOST_SPIRIT_DEFINE(floating_point_literal)

	const integer_literal_type integer_literal = "number (integer)";
	const auto integer_literal_def = x3::int_;
	BOOST_SPIRIT_DEFINE(integer_literal)

	const string_literal_type string_literal = "string";
	const auto string_literal_def = x3::lexeme['"' > *(x3::char_ - (x3::lit('"') | '\n' | '\r')) > '"'];
	BOOST_SPIRIT_DEFINE(string_literal)

	const socket_spec_literal_type socket_spec_literal = "socket spec literal";
	const auto socket_spec_literal_def = x3::lexeme[-common::integer_literal >> common::identifier];
	BOOST_SPIRIT_DEFINE(socket_spec_literal)

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

	const temp_literal_type temp_literal = "temp literal";
	const auto temp_literal_def = make_keyword(lang::keywords::rf::temp) > x3::attr(ast::common::temp_literal{});
	BOOST_SPIRIT_DEFINE(temp_literal)

	const none_literal_type none_literal = "none literal";
	const auto none_literal_def = make_keyword(lang::keywords::rf::none) > x3::attr(ast::common::none_literal{});
	BOOST_SPIRIT_DEFINE(none_literal)

	// ---- expressions ----

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
		| common::temp_literal
		| common::none_literal
		// 1. Must be attempted before integer literal, otherwise integer literal for eg "5GGG" would
		// leave letters which immediately follow (no whitespace) that would trigger parse error
		// 2. Must be attempted after all keyword literals, otherwise they will be incorrectly
		// attempted as a socket spec and error as they do not consist only of R/G/B/W/A/D letters
		| common::socket_spec_literal
		// Must be attempted before integer literal. Otherwise integer literal will just consume
		// the number and do not check if there is any . character following it.
		| common::floating_point_literal
		| common::integer_literal
		| common::string_literal;
	BOOST_SPIRIT_DEFINE(literal_expression)

	const comparison_operator_type comparison_operator = "comparison operator";
	const auto comparison_operator_def = symbols::rf::comparison_operators;
	BOOST_SPIRIT_DEFINE(comparison_operator)

	const comparison_expression_type comparison_expression = "comparison expression";
	const auto comparison_expression_def =
		// lexeme because
		// "< 3" has to be parsed as comparison(<, null) + value(3)
		// "<3"  has to be parsed as comparison(<, 3)
		x3::lexeme[comparison_operator > -integer_literal]
		// when on the implicit default (=), any integer that follows should be a value (not a part of the comparison)
		| (
			x3::attr(ast::common::comparison_operator::implicit_default())
			> x3::attr(boost::optional<ast::common::integer_literal>{})
		);
	BOOST_SPIRIT_DEFINE(comparison_expression)

	// Blacklist approach turned out to be very unstable (frequent endless loop or recursion). Thus,
	// approaches such as x3::lexeme[+(!x3::space - x3::lit('#') - x3::lit('{') - x3::lit('}') - x3::eoi)]
	// are discouraged due to instability and maintenance effort (need to take care of all special tokens).
	// Whitelist approach will not accept that much but is stable and easy to understand and test.
	const unknown_expression_type unknown_expression = "unknown expression";
	const auto unknown_expression_def = x3::lexeme[+(x3::alnum | x3::char_('_'))];
	BOOST_SPIRIT_DEFINE(unknown_expression)

	// ---- conditions ----

	// (none)

	// ---- actions ----

	// (none)

	// ---- statements ----

	const static_visibility_statement_type static_visibility_statement = "static visibility statement";
	const auto static_visibility_statement_def = make_keyword(symbols::rf::static_visibility_literals);
	BOOST_SPIRIT_DEFINE(static_visibility_statement)

	const continue_statement_type continue_statement = "continue statement";
	const auto continue_statement_def = make_keyword(lang::keywords::rf::continue_);
	BOOST_SPIRIT_DEFINE(continue_statement)

	const import_statement_type import_statement = "import statement";
	const auto import_statement_def =
		make_keyword(lang::keywords::rf::import_)
		> string_literal
		> ((make_keyword(lang::keywords::rf::optional) > x3::attr(true)) | x3::attr(false));
	BOOST_SPIRIT_DEFINE(import_statement)
} // namespace common

namespace rf
{
	using common::make_keyword;

	// ---- literal types ----

	const string_type string = "string";
	const auto string_def = common::string_literal | common::identifier;
	BOOST_SPIRIT_DEFINE(string)

	// ---- expressions ----

	const literal_sequence_type literal_sequence = "literal sequence";
	const auto literal_sequence_def = x3::skip(common::non_eol_whitespace)[+common::literal_expression];
	BOOST_SPIRIT_DEFINE(literal_sequence)

	// ---- conditions ----

	const condition_type condition = "condition";
	const auto condition_def =
		make_keyword(symbols::rf::official_condition_properties)
		> common::comparison_expression
		> rf::literal_sequence;
	BOOST_SPIRIT_DEFINE(condition)

	// ---- actions ----

	const action_type action = "action";
	const auto action_def = make_keyword(symbols::rf::official_action_properties) > rf::literal_sequence;
	BOOST_SPIRIT_DEFINE(action)

	// ---- filter structure ----

	const rule_type rule = "rule";
	const auto rule_def = condition | action | common::continue_statement;
	BOOST_SPIRIT_DEFINE(rule)

	const filter_block_type filter_block = "filter block";
	const auto filter_block_def = common::static_visibility_statement > *rule;
	BOOST_SPIRIT_DEFINE(filter_block)

	const block_variant_type block_variant = "block variant";
	const auto block_variant_def = common::import_statement | filter_block;
	BOOST_SPIRIT_DEFINE(block_variant)

	const grammar_type grammar = "code";
	const auto grammar_def = *block_variant > x3::eoi;
	BOOST_SPIRIT_DEFINE(grammar)
} // namespace rf

namespace sf
{
	using common::make_keyword;

	// ---- fundamental tokens ----

	const name_type name = "name";
	const auto name_def = x3::lexeme['$' >> common::identifier];
	BOOST_SPIRIT_DEFINE(name)

	// ---- literal types ----

	// (no spirit-filter-specific literal types)

	// ---- expressions ----

	const primitive_value_type primitive_value = "primitive";
	const auto primitive_value_def = name | common::literal_expression | common::unknown_expression;
	BOOST_SPIRIT_DEFINE(primitive_value)

	const sequence_type sequence = "list of values";
	const auto sequence_def = common::make_sequence(primitive_value);
	BOOST_SPIRIT_DEFINE(sequence)

	// moved here due to circular dependency
	const statement_type statement = "statement";

	const statement_list_expression_type statement_list_expression = "statement list expression";
	const auto statement_list_expression_def = ('{' >> *statement) > '}';
	BOOST_SPIRIT_DEFINE(statement_list_expression)

	const value_expression_type value_expression = "expression";
	const auto value_expression_def = statement_list_expression | sequence;
	BOOST_SPIRIT_DEFINE(value_expression)

	// ---- definitions ----

	const constant_definition_type constant_definition = "constant definiton";
	const auto constant_definition_def = (name >> '=') > value_expression;
	BOOST_SPIRIT_DEFINE(constant_definition)

	// future space for metedata definitions
	const definition_type definition = "definition";
	const auto definition_def = constant_definition;
	BOOST_SPIRIT_DEFINE(definition)

	// ---- conditions ----

	const autogen_condition_type autogen_condition = "autogen condition";
	const auto autogen_condition_def = make_keyword(lang::keywords::sf::autogen) > sequence;
	BOOST_SPIRIT_DEFINE(autogen_condition)

	const price_comparison_condition_type price_comparison_condition = "price comparison condition";
	const auto price_comparison_condition_def =
		make_keyword(lang::keywords::sf::price)
		> common::comparison_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(price_comparison_condition)

	const official_condition_type official_condition = "condition (official)";
	const auto official_condition_def =
		make_keyword(symbols::rf::official_condition_properties)
		> common::comparison_expression
		> sequence;
	BOOST_SPIRIT_DEFINE(official_condition)

	const condition_type condition = "condition";
	const auto condition_def =
		  autogen_condition
		| price_comparison_condition
		| official_condition;
	BOOST_SPIRIT_DEFINE(condition)

	// ---- actions ----

	const set_alert_sound_action_type set_alert_sound_action = "set alert sound action";
	const auto set_alert_sound_action_def = make_keyword(lang::keywords::sf::set_alert_sound) > sequence;
	BOOST_SPIRIT_DEFINE(set_alert_sound_action)

	const official_action_type official_action = "action (official)";
	const auto official_action_def = make_keyword(symbols::rf::official_action_properties) > sequence;
	BOOST_SPIRIT_DEFINE(official_action)

	const action_type action = "action";
	const auto action_def = official_action | set_alert_sound_action;
	BOOST_SPIRIT_DEFINE(action)

	// ---- filter structure ----

	const expand_statement_type expand_statement = "expand statement";
	const auto expand_statement_def = make_keyword(lang::keywords::sf::expand) > sequence;
	BOOST_SPIRIT_DEFINE(expand_statement)

	const dynamic_visibility_policy_type dynamic_visibility_policy = "dynamic visivility policy";
	const auto dynamic_visibility_policy_def = make_keyword(symbols::sf::dynamic_visibility_literals);
	BOOST_SPIRIT_DEFINE(dynamic_visibility_policy)

	const dynamic_visibility_statement_type dynamic_visibility_statement = "dynamic visibility statement";
	const auto dynamic_visibility_statement_def = dynamic_visibility_policy > sequence;
	BOOST_SPIRIT_DEFINE(dynamic_visibility_statement)

	const visibility_statement_type visibility_statement = "visibility statement";
	const auto visibility_statement_def = common::static_visibility_statement | dynamic_visibility_statement;
	BOOST_SPIRIT_DEFINE(visibility_statement)

	const behavior_statement_type behavior_statement = "behavior statement";
	const auto behavior_statement_def = visibility_statement > -common::continue_statement;
	BOOST_SPIRIT_DEFINE(behavior_statement)

	const unknown_statement_type unknown_statement = "unknown/invalid statement";
	const auto unknown_statement_def = common::identifier > common::comparison_expression > sequence;
	BOOST_SPIRIT_DEFINE(unknown_statement)

	// moved here due to circular dependency
	const rule_block_type rule_block = "rule block";

	const auto statement_def =
		common::import_statement | condition | action | expand_statement | behavior_statement | rule_block | unknown_statement;
	BOOST_SPIRIT_DEFINE(statement)

	const auto rule_block_def = x3::lit('{') > *statement > x3::lit('}');
	BOOST_SPIRIT_DEFINE(rule_block)

	const filter_structure_type filter_structure = "filter structure";
	const auto filter_structure_def = *definition > *statement;
	BOOST_SPIRIT_DEFINE(filter_structure)

	const grammar_type grammar = "code";
	const auto grammar_def = filter_structure > x3::eoi;
	BOOST_SPIRIT_DEFINE(grammar)

} // namespace sf

} // namespace fs::parser::detail
