#include "fs/print/generic.hpp"

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
	{
		iterator_type new_end = range.end();
		return range_type(it, ++new_end);
	}

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

void print_line_number(logger& logger, int line)
{
	logger << "line " << line << ": ";
}

void print_line(logger& logger, iterator_type start, iterator_type last)
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

	logger << range_type(start, end) << '\n';
}

void print_indicator(logger& logger, iterator_type line_start, range_type error_range)
{
	for (; line_start != error_range.begin(); ++line_start)
	{
		auto c = *line_start;
		if (c == '\r' || c == '\n')
			break;
		else if (c == '\t')
			for (int i = 0; i < tab_length; ++i)
				logger << indicator_background;
		else
			logger << indicator_background;
	}

	for (auto it = error_range.begin(); it != error_range.end(); ++it)
	{
		logger << indicator_foreground;
	}

	logger << '\n';
}

void print_line_with_indicator(logger& logger, range_type content_range, range_type error_range)
{
	iterator_type line_start = get_line_start(content_range.begin(), error_range.begin());
	if (line_start != content_range.begin())
		++line_start;

	print_line(logger, line_start, content_range.end());
	print_indicator(logger, line_start, error_range);
}

void print_internal_error(logger& logger, std::string_view desc)
{
	logger.error() << internal_error_string << desc;
}

}
