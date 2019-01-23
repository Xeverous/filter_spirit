#pragma once
#include "print/generic.hpp"

namespace fs::print
{

template <typename OutputStream>
void parse_error(
	OutputStream& os,
	range_type content,
	iterator_type err_pos,
	std::string_view error_message)
{
	const iterator_type first = content.begin();
	const iterator_type last = content.end();

	// make sure err_pos does not point to white space
	err_pos = skip_whitespace(err_pos, last);

	print_line_number(os, count_line_number(first, err_pos));
	os << error_message;

	iterator_type start = get_line_start(first, err_pos);
	if (start != first)
		++start;
	print_line(os, start, last);
	print_indicator(os, start, err_pos, '_');
	os << "^_";
}

template <typename OutputStream>
void parse_error(
	OutputStream& os,
	range_type content,
	range_type error_range,
	std::string_view error_message)
{
	const iterator_type first = content.begin();
	const iterator_type last = content.end();

	// make sure err_pos does not point to white space
	const iterator_type err_first = skip_whitespace(error_range.begin(), last);

	print_line_number(os, count_line_number(first, err_first));
	os << error_message;

	iterator_type start = get_line_start(first, err_first);
	if (start != first)
		++start;
	print_line(os, start, last);
	print_indicator(os, start, err_first, ' ');
	print_indicator(os, start, error_range.end(), '~');
	os << " <<-- Here";
}

}
