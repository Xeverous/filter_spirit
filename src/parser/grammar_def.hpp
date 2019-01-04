/**
 * @file grammar type definitions
 *
 * @details This file is intended for core grammar definitions.
 * Use BOOST_SPIRIT_DEFINE here.
 */
#pragma once
#include "grammar.hpp"

namespace fs::parser
{

const comment_type comment = "comment";
const auto comment_def = lexeme['#' >> *(char_ - newline_character)];
BOOST_SPIRIT_DEFINE(comment)

const identifier_type identifier = "identifier";
const auto identifier_def = lexeme[(alpha | '_') >> *(alnum | '_')];
BOOST_SPIRIT_DEFINE(identifier)

const whitespace_type whitespace = "whitespace";
const auto whitespace_def = x3::space - newline_character;
BOOST_SPIRIT_DEFINE(whitespace)

const string_type string = "string";
const auto string_def = lexeme['"' >> +(char_ - '"') >> '"'];
BOOST_SPIRIT_DEFINE(string)

const constant_boolean_definition_type constant_boolean_definition = "Boolean definition";
const auto constant_boolean_definition_def = lit(keyword_boolean) >> identifier[set_first] >> lit(assignment_operator) >> booleans[set_second];
BOOST_SPIRIT_DEFINE(constant_boolean_definition)

const constant_number_definition_type constant_number_definition = "Number definition";
const auto constant_number_definition_def = lit(keyword_number) >> identifier[set_first] >> lit(assignment_operator) >> x3::int_[set_second];
BOOST_SPIRIT_DEFINE(constant_number_definition)

const constant_definition_type constant_definition = "value definition";
const auto constant_definition_def = constant_boolean_definition | constant_number_definition;
BOOST_SPIRIT_DEFINE(constant_definition)

const code_line_type code_line = "line";
const auto code_line_def = newline_character | comment | constant_definition;
BOOST_SPIRIT_DEFINE(code_line)

const grammar_type grammar = "code";
const auto grammar_def = *code_line;
BOOST_SPIRIT_DEFINE(grammar)

}

namespace fs
{

parser::grammar_type grammar()
{
	return parser::grammar;
}

parser::skipper_type skipper()
{
	return parser::whitespace;
}

}
