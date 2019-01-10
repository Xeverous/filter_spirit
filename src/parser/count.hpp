/*
 * count parser
 * custom parser based on x3::matches[a]
 *
 * count[a] will attempt to parse a multiple times
 * and expose the count as it's attribute - this parser
 * can not fail, will at worst return 0
 *
 * https://stackoverflow.com/questions/54128284
 */
#pragma once
#include <boost/spirit/home/x3/core/parser.hpp>
#include <boost/spirit/home/x3/support/traits/move_to.hpp>
#include <boost/spirit/home/x3/support/unused.hpp>

namespace x3ext
{

template<typename Subject>
struct count_directive: boost::spirit::x3::unary_parser<Subject, count_directive<Subject>>
{
	using base_type = boost::spirit::x3::unary_parser<Subject, count_directive<Subject>>;
	static bool const has_attribute = true;
	using attribute_type = int;

	count_directive(Subject const& subject)
		: base_type(subject)
	{
	}

	template<typename Iterator, typename Context, typename RContext, typename Attribute>
	bool parse(
		Iterator& first,
		Iterator const& last,
		Context const& context,
		RContext& rcontext,
		Attribute& attr) const
	{
		int count = 0;
		while (this->subject.parse(first, last, context, rcontext, boost::spirit::x3::unused))
		{
			++count;
		}
		boost::spirit::x3::traits::move_to(count, attr);
		return true;
	}
};

struct count_gen
{
	template<typename Subject>
	count_directive<typename boost::spirit::x3::extension::as_parser<Subject>::value_type>
	operator[](Subject const& subject) const
	{
		return { boost::spirit::x3::as_parser(subject) };
	}
};

auto const count = count_gen{};

}
