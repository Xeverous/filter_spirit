#include "fs/log/utility.hpp"

#include <string_view>
#include <cassert>

namespace fs::log
{

int count_lines(const char* first, const char* last)
{
	int line = 1;
	char prev = '\0';

	for (; first != last; ++first)
	{
		const char c = *first;
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

std::string_view make_string_view(const char* first, const char* last)
{
	assert(first <= last);

	if (first == last)
		return std::string_view();

	return std::string_view(first, last - first);
}

}
