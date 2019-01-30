#pragma once
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <string>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;

using whitespace_type = x3::rule<struct whitespace_class>;
using skipper_type = whitespace_type;

using iterator_type = std::string::const_iterator;
using range_type = boost::iterator_range<iterator_type>;
using position_cache_type = x3::position_cache<std::vector<iterator_type>>;
using phrase_context_type = typename x3::phrase_parse_context<skipper_type>::type;
using inner_context_type = x3::context<struct position_cache_tag, std::reference_wrapper<position_cache_type>, phrase_context_type>;
using context_type = x3::context<struct error_stream_tag, std::reference_wrapper<std::ostream>, inner_context_type>;

}
