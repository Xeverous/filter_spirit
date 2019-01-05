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
#include <tuple>
#include <utility>
#include <functional>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;

// tag used to get the position cache from the context
struct position_cache_tag;

// Inherit rule ID type from this type if you want Spirit to call on_success/on_error
// this allows the same things as expr[func] in grammar definitions but does not bloat it.
// This type can be used for any rule that has x3::position_tagged attribute type - it just
// fills it with source code information (eg for later semantic analysis)
struct annotate_position
{
	template <typename T, typename Iterator, typename Context>
	void on_success(Iterator first, Iterator last, T& ast, const Context& context)
	{
		auto& position_cache = x3::get<position_cache_tag>(context).get();
		position_cache.annotate(ast, first, last);
	}
};

// rule IDs
struct comment_class {};
struct identifier_class : annotate_position {};
struct whitespace_class {};
struct string_literal_class : annotate_position {};
struct constant_boolean_definition_class {};
struct constant_number_definition_class {};
struct constant_level_definition_class {};
struct constant_sound_id_definition_class {};
struct constant_volume_definition_class {};

// whitespace
// Filter Spirit grammar skips any whitespace except newline character
using whitespace_type = x3::rule<whitespace_class>;
BOOST_SPIRIT_DECLARE(whitespace_type)

// comment - a line that starts with #
using comment_type = x3::rule<comment_class>;
BOOST_SPIRIT_DECLARE(comment_type)

// identifier
using identifier_type = x3::rule<identifier_class, std::string>;
BOOST_SPIRIT_DECLARE(identifier_type)

// string
using string_literal_type = x3::rule<class string_literal_class, std::string>;
BOOST_SPIRIT_DECLARE(string_literal_type)

// boolean definition: Boolean b = True
using constant_boolean_definition_type = x3::rule<class constant_boolean_definition_class, ast::constant_boolean_definition>;
BOOST_SPIRIT_DECLARE(constant_boolean_definition_type)

// number definition: Number n = 3
using constant_number_definition_type = x3::rule<class constant_number_definition_class, ast::constant_number_definition>;
BOOST_SPIRIT_DECLARE(constant_number_definition_type)

// Level definition: Level l = 3
using constant_level_definition_type = x3::rule<class constant_level_definition_class, ast::constant_level_definition>;
BOOST_SPIRIT_DECLARE(constant_level_definition_type)

// SoundId definition: SoundId si = 3
using constant_sound_id_definition_type = x3::rule<class constant_sound_id_definition_class, ast::constant_sound_id_definition>;
BOOST_SPIRIT_DECLARE(constant_sound_id_definition_type)

// Volume definition: Volume v = 300
using constant_volume_definition_type = x3::rule<class constant_volume_definition_class, ast::constant_volume_definition>;
BOOST_SPIRIT_DECLARE(constant_volume_definition_type)

// constants
using constant_definition_type = x3::rule<class constant_definition_class, boost::variant<
	constant_boolean_definition_type::attribute_type,
	constant_number_definition_type::attribute_type,
	constant_level_definition_type::attribute_type,
	constant_sound_id_definition_type::attribute_type,
	constant_volume_definition_type::attribute_type
>>;
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
using position_cache = boost::spirit::x3::position_cache<std::vector<iterator_type>>; // given AST, allows lookup of source code places
using phrase_context_type = x3::phrase_parse_context<skipper_type>::type;
using context_type = x3::context<position_cache_tag, std::reference_wrapper<position_cache>, phrase_context_type>;

}

namespace fs::ast
{

using ast_type = fs::parser::grammar_type::attribute_type;

}

namespace fs
{

parser::grammar_type grammar();
parser::skipper_type skipper();

}
