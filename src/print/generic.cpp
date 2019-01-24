#include "print/generic.hpp"
#include <cctype>
#include <iterator>

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

range_type skip_whitespace(range_type range, iterator_type last)
{
	iterator_type it = range.begin();
	while (it != range.end() && it != last)
	{
		const auto c = *it;
		if (std::isspace(static_cast<unsigned char>(c)))
			++it;
		else
			break;
	}

	// make sure we havent accidentally advanced range beginning so that the range is empty
	// move end 1 position forward unless last has been reached
	if (it == range.end() && range.end() != last)
		return range_type(it, ++range.end());
	else
		return range_type(it, range.end());

}

iterator_type get_line_start(iterator_type first, iterator_type pos)
{
	iterator_type latest = first;
	for (iterator_type it = first; it != pos; ++it)
		if (*it == '\r' || *it == '\n')
			latest = it;
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
