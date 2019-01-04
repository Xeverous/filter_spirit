/**
 * @file grammar type definitions
 *
 * @details This file is intended for main parser API.
 * This is the file that all parser source files should include.
 * Use BOOST_SPIRIT_DECLARE here.
 */
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
BOOST_SPIRIT_DECLARE(comment_type)

// identifier
using identifier_type = x3::rule<class identifier_class, std::string>;
BOOST_SPIRIT_DECLARE(identifier_type)

// whitespace
// Filter Spirit grammar skips any whitespace except newline character
using whitespace_type = x3::rule<class whitespace_class>;
BOOST_SPIRIT_DECLARE(whitespace_type)

// string
using string_type = x3::rule<class string_class, std::string>;
BOOST_SPIRIT_DECLARE(string_type)

// boolean definition: Boolean b = True
using constant_boolean_definition_type = x3::rule<class constant_boolean_definition_class, std::pair<std::string, bool>>;
BOOST_SPIRIT_DECLARE(constant_boolean_definition_type)

// number definition: Number n = 3
using constant_number_definition_type = x3::rule<class constant_number_definition_class, std::pair<std::string, int>>;
BOOST_SPIRIT_DECLARE(constant_number_definition_type)

// constants
using constant_definition_type = x3::rule<class constant_definition_class, boost::variant<constant_boolean_definition_type::attribute_type, constant_number_definition_type::attribute_type>>;
BOOST_SPIRIT_DECLARE(constant_definition_type)

// filter language consists of lines, of which every is a comment or some code
using code_line_type = x3::rule<class code_line_class, boost::optional<constant_definition_type::attribute_type>>;
BOOST_SPIRIT_DECLARE(code_line_type)

// the entire language grammar
using grammar_type = x3::rule<class grammar_class, std::vector<code_line_type::attribute_type>>;
BOOST_SPIRIT_DECLARE(grammar_type)

using skipper_type = whitespace_type;

// Boost Spirit recommends that this should be in a separate config.hpp file but
// in our case we need skipper type to be visible so we place configuration here
using iterator_type = std::string::const_iterator;
using context_type = x3::phrase_parse_context<skipper_type>::type;


}

namespace fs
{

parser::grammar_type grammar();
parser::skipper_type skipper();

}
