#include "fs/log/logger.hpp"
#include "fs/log/utility.hpp"

#include <cassert>

namespace
{

/**
 * @brief find first character after last line break in range [@p first, @p last)
 * @return iterator pointing to first-of-line character or first if no line break was found
 * @details supports Windows (CRLF) and Unix (LF) endlines
 */
[[nodiscard]]
const char* find_line_beginning_backward(const char* first, const char* last)
{
	const char* it = last;
	while (last-- != first)
	{
		if (*last == '\n')
			break;
		else
			it = last;
	}

	return it;
}
/**
 * @brief find last character before first line break in range [@p first, @p last)
 * @return iterator pointing to '\n' or last if no line break was found
 * @details supports any line break format
 */
[[nodiscard]]
const char* find_line_end(const char* first, const char* last)
{
	for (; first != last; ++first)
	{
		if (*first == '\n' || *first == '\r')
			return first;
	}

	return last;
}

/**
 * @brief skip characters as long as one of Chars... is encountered or until @p last is reached
 */
template <char... Chars> [[nodiscard]]
const char* skip(const char* first, const char* last)
{
	while (first != last)
	{
		if (((*first != Chars) && ... && true))
			break;

		++first;
	}

	return first;
}

[[nodiscard]] const char* skip_lf_cr (const char* first, const char* last) { return skip<'\n', '\r'>(first, last); }
[[nodiscard]] const char* skip_indent(const char* first, const char* last) { return skip<'\t', ' '> (first, last); }

} // namespace

namespace fs::log
{

logger_wrapper::~logger_wrapper()
{
	logger.end_message();
}

void logger::print_underlined_code(std::string_view all_code, std::string_view underlined_code)
{
	const char *const all_first = all_code.data();
	const char *const all_last  = all_code.data() + all_code.size();

	const char *const underline_first = underlined_code.data();
	const char *const underline_last  = underlined_code.data() + underlined_code.size();

	const char *const code_first = find_line_beginning_backward(all_first, underline_first);
	const char *const code_last  = find_line_end(underline_last, all_last);

	logger& self = *this;
	const char* code_it = code_first;
	const char* underline_it = underline_first;
	while (code_it != code_last)
	{
		const char *const code_line_first = code_it;
		const char *const code_line_last  = find_line_end(code_line_first, all_last);
		self << make_string_view(code_line_first, code_line_last) << '\n';

		const char *const indent_first = code_line_first;
		const char *const indent_last  = skip_indent(code_line_first, code_line_last);
		self << make_string_view(indent_first, indent_last);

		const char *const non_underline_first = indent_last;
		const char *const non_underline_last  = underline_it;
		for (auto it = non_underline_first; it != non_underline_last; ++it)
			self << ' ';

		const char *const underline_line_first = non_underline_last;
		const char *const underline_line_last  = find_line_end(underline_line_first, underline_last);
		for (auto it = underline_line_first; it != underline_line_last; ++it)
			self << '~';

		self << '\n';

		code_it      = skip_lf_cr(code_line_last, code_last);
		underline_it = skip_lf_cr(underline_line_last, underline_last);
	}
}

void logger::internal_error(std::string_view description)
{
	error() << "internal error: " << description;
}

}
