#pragma once
#include "parser/config.hpp"

namespace fs::print
{

using iterator_type = parser::iterator_type;
using range_type = parser::range_type;
constexpr auto tab_length = 8;

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

	os << range_type(start, end);
}

template <typename OutputStream>
void print_indicator(OutputStream& os, iterator_type& start, iterator_type last, char ind)
{
	for (; start != last; ++start)
	{
		auto c = *start;
		if (c == '\r' || c == '\n')
			break;
		else if (c == '\t')
			for (int i = 0; i < tab_length; ++i)
				os << ind;
		else
			os << ind;
	}
}

}
