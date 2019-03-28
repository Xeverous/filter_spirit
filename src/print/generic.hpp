#pragma once

#include "parser/config.hpp"
#include "log/logger.hpp"

#include <utility>

namespace fs::print
{

using iterator_type = parser::iterator_type;
using range_type = parser::range_type;
constexpr auto tab_length = 8;
constexpr auto indicator_background = ' ';
constexpr auto indicator_foreground = '~';
constexpr auto compiler_error_string = "compile error: ";
constexpr auto internal_compiler_error_string = "internal compile error: ";
constexpr auto internal_compiler_error_info = "please report a bug with attached minimal filter source that reproduces it";
constexpr auto note_string = "note: ";

/**
 * @param first beginning of the input, or something further
 * @param pos position of interest
 * @return iterator to first character of the line where pos is
 *
 * @note function assumes that first <= pos
 */
[[nodiscard]]
iterator_type get_line_start(iterator_type first, iterator_type pos);
/**
 * @param first beginning of the input
 * @param it position of interest
 * @return amount of encountered line breaks + 1
 */
[[nodiscard]]
int count_line_number(iterator_type first, iterator_type it);

/**
 * Move 'it' (or 'range') forward so that it does not point at whitespace by
 * skipping until hit a non-whitespace or reached 'last'. In case range would be
 * of length 0 it's end is moved forward 1 position if it would not exceed 'last'
 */
[[nodiscard]]
iterator_type skip_whitespace(iterator_type it, iterator_type last);
[[nodiscard]]
range_type skip_whitespace(range_type range, iterator_type last);


void print_line_number(logger& logger, int line);
void print_line(logger& logger, iterator_type start, iterator_type last);
void print_indicator(logger& logger, iterator_type line_start, range_type error_range);
void print_line_with_indicator(logger& logger, range_type content_range, range_type error_range);

template <typename... Texts>
void print_line_number_with_indication_and_texts(
	logger& logger,
	range_type content_range,
	range_type indicated_range,
	Texts&&... texts)
{
	/*
	 * Make sure error does not point to whitespace - such errors would be rather unclear for the user.
	 * Why it happens? I don't know, even boost::spirit::x3 implementation uses whitespace skipping in
	 * their examples - perhaps whitespace are just the place where parser backtracks upon encountering
	 * an error. So move the iterator forward then to express that the error was caused by next token.
	 */
	indicated_range = skip_whitespace(indicated_range, content_range.end());
	const int line_number = count_line_number(content_range.begin(), indicated_range.begin());

	print_line_number(logger, line_number);
	(logger << ... << std::forward<Texts>(texts)) << '\n';
	print_line_with_indicator(logger, content_range, indicated_range);
}

}
