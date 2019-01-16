#include "parser/error_printer.hpp"
#include <cctype>

namespace fs::parser
{

void error_printer::skip_whitespace(iterator_type& err_pos, iterator_type last) const
{
	while (err_pos != last)
	{
		char c = *err_pos;
		// Note: it is UB to pass chars that are not representable as unsigned char
		// https://en.cppreference.com/w/cpp/string/byte/isspace
		if (std::isspace(static_cast<unsigned char>(c)))
			++err_pos;
		else
			break;
	}
}

void error_printer::skip_non_whitespace(iterator_type& err_pos, iterator_type last) const
{
	while (err_pos != last)
	{
		char c = *err_pos;
		if (std::isspace(static_cast<unsigned char>(c)))
			break;
		else
			++err_pos;
	}
}

iterator_type error_printer::get_line_start(iterator_type first, iterator_type pos) const
{
	iterator_type latest = first;
	for (iterator_type i = first; i != pos; ++i)
		if (*i == '\r' || *i == '\n')
			latest = i;
	return latest;
}

std::size_t error_printer::position(iterator_type first, iterator_type it) const
{
	int line = 1;
	typename std::iterator_traits<iterator_type>::value_type prev{ 0 };

	for (iterator_type pos = first; pos != it; ++pos)
	{
		const auto c = *pos;
		switch (c)
		{
			case '\n':
				if (prev != '\r')
					++line;
				break;
			case '\r':
				++line;
				break;
			default:
				break;
		}
		prev = c;
	}

	return line;
}

}
