#pragma once
#include "parser/config.hpp"
#include <ostream>

namespace fs::parser
{

// code 'stolen' from boost::spirit::x3::error_handler for customization reasons
// TODO: refactor to free functions
class error_printer
{
public:
	using result_type = void;
	static constexpr auto tab_length = 8;

	template <typename OutputStream>
	void parse_error(
		OutputStream& os,
		range_type content,
		iterator_type err_pos,
		std::string_view error_message);

	template <typename OutputStream>
	void parse_error(
		OutputStream& os,
		range_type content,
		range_type error_range,
		std::string_view error_message);

private:

	template <typename OutputStream>
	void print_line_number(OutputStream& os, int line) const;
	template <typename OutputStream>
	void print_line(OutputStream& os, iterator_type line_start, iterator_type last) const;
	template <typename OutputStream>
	void print_indicator(OutputStream& os, iterator_type& line_start, iterator_type last, char ind) const;

	void skip_whitespace(iterator_type& err_pos, iterator_type last) const;
	void skip_non_whitespace(iterator_type& err_pos, iterator_type last) const;

	iterator_type get_line_start(iterator_type first, iterator_type pos) const;
	std::size_t position(iterator_type first, iterator_type it) const;
};

template <typename OutputStream>
void error_printer::print_line_number(OutputStream& os, int line) const
{
	os << "line " << line << ": ";
}

template <typename OutputStream>
void error_printer::print_line(OutputStream& os, iterator_type start, iterator_type last) const
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
void error_printer::print_indicator(OutputStream& os, iterator_type& start, iterator_type last, char ind) const
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

template <typename OutputStream>
void error_printer::parse_error(
	OutputStream& os,
	range_type content,
	iterator_type err_pos,
	std::string_view error_message)
{
	const iterator_type first = content.begin();
	const iterator_type last = content.end();

	// make sure err_pos does not point to white space
	skip_whitespace(err_pos, last);

	print_line_number(os, position(first, err_pos));
	os << error_message;

	iterator_type start = get_line_start(first, err_pos);
	if (start != first)
		++start;
	print_line(os, start, last);
	print_indicator(os, start, err_pos, '_');
	os << "^_";
}

template <typename OutputStream>
void error_printer::parse_error(
	OutputStream& os,
	range_type content,
	range_type error_range,
	std::string_view error_message)
{
	const iterator_type first = content.begin();
	const iterator_type last = content.end();

	// make sure err_pos does not point to white space
	iterator_type err_first = error_range.begin();
	skip_whitespace(err_first, last);

	print_line_number(os, position(first, err_first));
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
