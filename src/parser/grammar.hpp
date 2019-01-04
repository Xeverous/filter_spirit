#pragma once
#include <boost/spirit/home/x3.hpp>
#include <tuple>
#include <utility>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;
// symbols that denote filter's language constants
struct booleans_ : x3::symbols<bool>
{
	booleans_()
	{
		add
			("True", true)
			("False", false)
		;
	}

};
const booleans_ booleans;


// some constants to aovid code duplication
constexpr auto keyword_boolean = "Boolean";
constexpr auto keyword_number  = "Number";
constexpr auto keyword_true    = "True";
constexpr auto keyword_false   = "False";
constexpr auto assignment_operator = '=';
constexpr auto newline_character   = '\n';

auto set_first = [](auto& ctx){ _val(ctx).first += _attr(ctx); };
auto set_second = [](auto& ctx){ _val(ctx).second += _attr(ctx); };

using x3::raw;
using x3::lexeme;
using x3::alpha;
using x3::alnum;
using x3::char_;
using x3::lit;

// Spirit coding style example:
// - rule ID        : foo_class
// - rule type      : foo_type
// - rule definition: foo_def
// - rule object    : foo

// comment - a line that starts with #
using comment_type = x3::rule<class comment_class>;
const comment_type comment = "comment";
const auto comment_def = lexeme['#' >> *(char_ - newline_character)];
BOOST_SPIRIT_DEFINE(comment)

// identifier
using identifier_type = x3::rule<class identifier_class, std::string>;
const identifier_type identifier = "identifier";
const auto identifier_def = lexeme[(alpha | '_') >> *(alnum | '_')];
BOOST_SPIRIT_DEFINE(identifier)

// whitespace
// Filter Spirit grammar skips any whitespace except newline character
using whitespace_type = x3::rule<class whitespace_class>;
const whitespace_type whitespace = "whitespace";
const auto whitespace_def = x3::space - newline_character;
BOOST_SPIRIT_DEFINE(whitespace)

// string
using string_type = x3::rule<class string_class, std::string>;
const string_type string = "string";
const auto string_def = lexeme['"' >> +(char_ - '"') >> '"'];
BOOST_SPIRIT_DEFINE(string)

// boolean definition: Boolean b = True
using constant_boolean_definition_type = x3::rule<class constant_boolean_definition_class, std::pair<std::string, bool>>;
const constant_boolean_definition_type constant_boolean_definition = "Boolean definition";
const auto constant_boolean_definition_def = lit(keyword_boolean) >> identifier[set_first] >> lit(assignment_operator) >> booleans[set_second];
BOOST_SPIRIT_DEFINE(constant_boolean_definition)

// number definition: Number n = 3
using constant_number_definition_type = x3::rule<class constant_number_definition_class, std::pair<std::string, int>>;
const constant_number_definition_type constant_number_definition = "Number definition";
const auto constant_number_definition_def = lit(keyword_number) >> identifier[set_first] >> lit(assignment_operator) >> x3::int_[set_second];
BOOST_SPIRIT_DEFINE(constant_number_definition)

// constants
using constant_definition_type = x3::rule<class constant_definition_class, boost::variant<constant_boolean_definition_type::attribute_type, constant_number_definition_type::attribute_type>>;
const constant_definition_type constant_definition = "value definition";
const auto constant_definition_def = constant_boolean_definition | constant_number_definition;
BOOST_SPIRIT_DEFINE(constant_definition)

// filter language consists of lines, of which every is a comment or some code
using code_line_type = x3::rule<class code_line_class, boost::optional<constant_definition_type::attribute_type>>;
const code_line_type code_line = "line";
const auto code_line_def = newline_character | comment | constant_definition;
BOOST_SPIRIT_DEFINE(code_line)

}
