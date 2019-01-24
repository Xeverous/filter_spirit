#pragma once
#include "print/generic.hpp"

namespace fs::print
{

template <typename OutputStream>
void parse_error(
	OutputStream& os,
	range_type content,
	range_type error_range,
	std::string_view expected)
{
	/*
	 * Make sure error does not point to whitespace - such errors would be rather unclear for the user.
	 * Why it happens? I don't know, even boost::spirit::x3 implementation uses whitespace skipping in
	 * their examples - perhaps whitespace are just the place where parser backtracks upon encountering
	 * an error. So move the iterator forward then to express that the error was caused by next token.
	 */
	error_range = skip_whitespace(error_range, content.end());

	print_line_number(os, count_line_number(content.begin(), error_range.begin()));
	os << "parse error: expected '" << expected << "' here\n";

	print_line_with_indicator(os, content, error_range);
}

template <typename OutputStream>
void parse_error(
	OutputStream& os,
	range_type content,
	iterator_type error_pos,
	std::string_view expected)
{
	const iterator_type error_first = error_pos;
	const iterator_type error_last = ++error_pos;

	parse_error(
		os,
		content,
		range_type(error_first, error_last),
		expected);
}

}
