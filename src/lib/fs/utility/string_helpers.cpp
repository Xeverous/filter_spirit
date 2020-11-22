#include <fs/utility/string_helpers.hpp>
#include <fs/utility/assert.hpp>

#include <algorithm>

namespace
{

/**
 * @brief find first character after last line break in range [@p first, @p last)
 * @return iterator pointing to first-of-line character or first if no line break was found
 * @details supports Windows (CRLF) and Unix (LF) endlines
 */
[[nodiscard]]
const char* find_line_beginning_backward(const char* first, const char* last) noexcept
{
	const char* it = last;
	while (last-- != first) {
		if (*last == '\n')
			break;
		else
			it = last;
	}

	return it;
}

/**
 * @brief skip characters as long as one of Chars... is encountered or until @p last is reached
 */
template <char... Chars> [[nodiscard]]
const char* skip(const char* first, const char* last) noexcept
{
	while (first != last) {
		// we could use an unary fold but the binary one allows empty template parameter list
		if (((*first != Chars) && ... && true))
			break;

		++first;
	}

	return first;
}

} // namespace

namespace fs::utility
{

const char* find_line_end(const char* first, const char* last) noexcept
{
	for (; first != last; ++first) {
		if (*first == '\n' || *first == '\r')
			return first;
	}

	return last;
}

const char* skip_eol(const char* first, const char* last) noexcept
{
	first = std::find(first, last, '\n');
	if (first != last)
		++first;

	return first;
}

const char* skip_lf_cr (const char* first, const char* last) noexcept { return skip<'\n', '\r'>(first, last); }
const char* skip_indent(const char* first, const char* last) noexcept { return skip<'\t', ' '> (first, last); }

std::string ptime_to_pretty_string(boost::posix_time::ptime time)
{
	if (time.is_special())
		return "(none)";

	std::string result = to_iso_extended_string(time.date()); // YYYY-MM-DD
	result += ' ';

	const auto append_num = [&](auto num) {
		if (num < 10)
			result += '0';

		result += std::to_string(num);
	};

	// HH:MM:SS
	append_num(time.time_of_day().hours());
	result += ':';
	append_num(time.time_of_day().minutes());
	result += ':';
	append_num(time.time_of_day().seconds());

	return result;
}

int count_lines(const char* first, const char* last) noexcept
{
	int line = 1;
	char prev = '\0';

	for (; first != last; ++first) {
		const char c = *first;
		switch (c) {
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

std::string_view make_string_view(const char* first, const char* last) noexcept
{
	FS_ASSERT(first <= last);

	if (first == last)
		return std::string_view();

	return std::string_view(first, last - first);
}

std::string_view ltrim(std::string_view str, char c)
{
	std::string_view::difference_type count = 0;

	for (char ch : str) {
		if (ch == c)
			++count;
		else
			break;
	}

	str.remove_prefix(count);
	return str;
}

std::string_view rtrim(std::string_view str, char c)
{
	std::string_view::difference_type count = 0;

	for (auto it = str.rbegin(); it != str.rend(); ++it) {
		if (*it == c)
			++count;
		else
			break;
	}

	str.remove_suffix(count);
	return str;
}

std::string_view trim(std::string_view str, char c)
{
	return rtrim(ltrim(str, c), c);
}

code_underliner::code_underliner(std::string_view all_code, std::string_view code_to_underline)
: all_first(all_code.data())
, all_last (all_code.data() + all_code.size())
, underline_first(code_to_underline.data())
, underline_last (code_to_underline.data() + code_to_underline.size())
, code_it   (find_line_beginning_backward(all_first, underline_first))
, code_last (find_line_end(underline_last, all_last))
{
}

code_underliner::code_underliner(std::string_view all_code, const char* character_to_underline)
: code_underliner(all_code, std::string_view(character_to_underline, 1u))
{
}

underlined_code code_underliner::next() noexcept
{
	if (code_it == code_last)
		return underlined_code{};

	const char *const code_line_first = code_it;
	const char *const code_line_last  = find_line_end(code_line_first, all_last);
	const auto code_line = utility::make_string_view(code_line_first, code_line_last);

	const char *const indent_first = code_line_first;
	const char *const indent_last  = skip_indent(code_line_first, code_line_last);
	const std::size_t indent = indent_last - indent_first;

	const auto spaces = [&]() -> std::size_t {
		if (indent_last < underline_first)
			return underline_first - indent_last;
		else
			return 0;
	}();

	const char *const underline_line_first = indent_last + spaces;
	const char *const underline_line_last  = find_line_end(underline_line_first, underline_last);
	const std::size_t underlines = underline_line_last - underline_line_first;

	code_it = skip_lf_cr(code_line_last, code_last);

	return underlined_code{code_line, indent, spaces, underlines};
}

std::string range_underline_to_string(
	std::string_view all_code,
	std::string_view code_to_underline)
{
	std::string result;
	// result.size() is always >= code_to_underline.size()
	result.reserve(code_to_underline.size());

	code_underliner cu(all_code, code_to_underline);

	for (auto underlined_code = cu.next(); underlined_code; underlined_code = cu.next()) {
		result.append(underlined_code.code_line);
		result += '\n';
		result.append(underlined_code.indent_view());
		result.append(underlined_code.spaces, ' ');
		result.append(underlined_code.underlines, '~');
		result += '\n';
	}

	return result;
}

}
