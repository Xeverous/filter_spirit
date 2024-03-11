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

[[nodiscard]] bool compare_strings_ignore_diacritics_impl(
	std::string_view value, std::string_view requirement, bool allow_starts_with_match)
{
	struct two_byte_letter
	{
		char ascii;
		unsigned char utf8_first_byte;
		unsigned char utf8_second_byte;
	};

	const auto letters = {
		two_byte_letter{'o', static_cast<unsigned char>(0xC3), static_cast<unsigned char>(0xB6)}, // ö
		two_byte_letter{'O', static_cast<unsigned char>(0xC3), static_cast<unsigned char>(0x96)}  // Ö
	};

	      auto req_it   = requirement.begin();
	const auto req_last = requirement.end();
	      auto val_it   = value.begin();
	const auto val_last = value.end();

	while (req_it != req_last && val_it != val_last) {
		if (*req_it == *val_it) {
			++req_it;
			++val_it;
			continue;
		}

		for (auto letter : letters) {
			if (*req_it == letter.ascii && static_cast<unsigned char>(*val_it) == letter.utf8_first_byte) {
				if (++val_it == val_last)
					return false;

				if (static_cast<unsigned char>(*val_it) == letter.utf8_second_byte) {
					++req_it;
					++val_it;
					continue;
				}
				else {
					return false;
				}
			}
		}

		return false;
	}

	FS_ASSERT_MSG(req_it == req_last || val_it == val_last, "at least one of iterators should hit end");

	if (val_it == val_last) {
		// both iterators hit end: exact match (both sequences exhausted)
		// only val it hit end: no match (requirement not exhausted)
		return req_it == req_last;
	}

	// only req it hit end: starts-with match (value starts with requirement)
	return allow_starts_with_match;
}

} // namespace

namespace fs::utility
{

bool compare_strings_ignore_diacritics(
	std::string_view value, std::string_view requirement, bool exact_match_required)
{
	if (requirement.size() > value.size())
		return false;

	if (exact_match_required)
		return compare_strings_ignore_diacritics_impl(value, requirement, false);

	while (value.size() >= requirement.size()) {
		if (compare_strings_ignore_diacritics_impl(value, requirement, true))
			return true;

		value.remove_prefix(1u);
	}

	return false;
}

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

	return std::string_view(first, static_cast<std::size_t>(last - first));
}

std::string_view ltrim(std::string_view str, char c)
{
	std::string_view::size_type count = 0;

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
	std::string_view::size_type count = 0;

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

code_underliner::code_underliner(std::string_view surrounding_code, std::string_view code_to_underline)
: surrounding_first(surrounding_code.data())
, surrounding_last (surrounding_code.data() + surrounding_code.size())
, underline_first(code_to_underline.data())
, underline_last (code_to_underline.data() + code_to_underline.size())
, code_it  (find_line_beginning_backward(surrounding_first, underline_first))
, code_last(find_line_end(underline_last, surrounding_last))
{
}

code_underliner::code_underliner(std::string_view surrounding_code, const char* character_to_underline)
: code_underliner(surrounding_code, std::string_view(character_to_underline, 1u))
{
}

underlined_code code_underliner::next() noexcept
{
	if (code_it == code_last)
		return underlined_code{};

	const char *const code_line_first = code_it;
	const char *const code_line_last  = find_line_end(code_line_first, surrounding_last);
	const auto code_line = utility::make_string_view(code_line_first, code_line_last);

	const char *const indent_first = code_line_first;
	const char *const indent_last  = skip_indent(code_line_first, code_line_last);
	const auto indent = indent_last - indent_first;

	const auto spaces = [&]() -> std::ptrdiff_t {
		if (indent_last < underline_first)
			return underline_first - indent_last;
		else
			return 0;
	}();

	const char *const underline_line_first = indent_last + spaces;
	const char *const underline_line_last  = find_line_end(underline_line_first, underline_last);
	const auto underlines = underline_line_last - underline_line_first;

	code_it = skip_lf_cr(code_line_last, code_last);

	return underlined_code{
		code_line,
		static_cast<std::size_t>(indent),
		static_cast<std::size_t>(spaces),
		static_cast<std::size_t>(underlines)
	};
}

std::string range_underline_to_string(
	std::string_view surrounding_code,
	std::string_view code_to_underline)
{
	std::string result;
	// result.size() is always >= code_to_underline.size()
	result.reserve(code_to_underline.size());

	code_underliner cu(surrounding_code, code_to_underline);

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
