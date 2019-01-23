#include "print/generic.hpp"
#include <cctype>

namespace fs::print
{

iterator_type skip_whitespace(iterator_type it, iterator_type last)
{
	while (it != last)
	{
		const auto c = *it;
		// Note: it is UB to pass chars that are not representable as unsigned char
		// https://en.cppreference.com/w/cpp/string/byte/isspace
		if (std::isspace(static_cast<unsigned char>(c)))
			++it;
		else
			break;
	}

	return it;
}

iterator_type skip_non_whitespace(iterator_type it, iterator_type last)
{
	while (it != last)
	{
		const auto c = *it;
		if (std::isspace(static_cast<unsigned char>(c)))
			break;
		else
			++it;
	}

	return it;
}

iterator_type get_line_start(iterator_type first, iterator_type pos)
{
	iterator_type latest = first;
	for (iterator_type i = first; i != pos; ++i)
		if (*i == '\r' || *i == '\n')
			latest = i;
	return latest;
}

int count_line_number(iterator_type first, iterator_type it)
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
