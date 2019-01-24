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
	// TODO: verify that this is needed
	// make sure err_pos does not point to white space
	// error_range.begin() = skip_whitespace(error_range.begin(), last);

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
