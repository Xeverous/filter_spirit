#pragma once
#include "parser/config.hpp"

namespace fs::print
{

using iterator_type = parser::iterator_type;
using range_type = parser::range_type;
constexpr auto tab_length = 8;
constexpr auto indicator_background = '_';
constexpr auto indicator_foreground = '^';

[[nodiscard]]
iterator_type get_line_start(iterator_type first, iterator_type pos);
[[nodiscard]]
int count_line_number(iterator_type first, iterator_type it);

[[nodiscard]]
iterator_type skip_whitespace(iterator_type it, iterator_type last);
[[nodiscard]]
iterator_type skip_non_whitespace(iterator_type it, iterator_type last);


template <typename OutputStream>
void print_line_number(OutputStream& os, int line)
{
	os << "line " << line << ": ";
}

template <typename OutputStream>
void print_line(OutputStream& os, iterator_type start, iterator_type last)
{
	auto end = start;
	while (end != last)
	{
		auto c = *end;
		if (c == '\r' || c == '\n')
			break;
		else
			++end;
	}

	os << range_type(start, end) << '\n';
}

template <typename OutputStream>
void print_indicator(OutputStream& os, iterator_type line_start, range_type error_range)
{
	for (; line_start != error_range.begin(); ++line_start)
	{
		auto c = *line_start;
		if (c == '\r' || c == '\n')
			break;
		else if (c == '\t')
			for (int i = 0; i < tab_length; ++i)
				os << indicator_background;
		else
			os << indicator_background;
	}

	for (auto it = error_range.begin(); it != error_range.end(); ++it)
	{
		os << indicator_foreground;
	}

	os << '\n';
}

template<typename OutputStream>
void print_line_with_indicator(OutputStream& os, range_type content_range, range_type error_range)
{
	iterator_type line_start = get_line_start(content_range.begin(), error_range.begin());
	if (line_start != content_range.begin())
		++line_start;

	print_line(os, line_start, content_range.end());
	print_indicator(os, line_start, error_range);
}

}
