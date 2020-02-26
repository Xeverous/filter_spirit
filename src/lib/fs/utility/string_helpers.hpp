#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <string_view>

namespace fs::utility
{

[[nodiscard]] std::string ptime_to_pretty_string(boost::posix_time::ptime time);

inline bool contains(std::string_view source, std::string_view fragment) noexcept
{
	return source.find(fragment) != std::string_view::npos;
}

// supports any line break style
[[nodiscard]] int count_lines(const char* first, const char* last) noexcept;

[[nodiscard]] std::string_view make_string_view(const char* first, const char* last) noexcept;

/**
 * A line with underline for pretty-printing
 *
 *     white = RGB([2, 5, 5], 255, 255)
 *                 ~~~~~~~~~
 */
struct underlined_code
{
	explicit operator bool() const noexcept { return !code_line.empty(); }
	bool operator!() const noexcept { return code_line.empty(); }

	std::string_view indent_view() const noexcept
	{
		return utility::make_string_view(code_line.data(), code_line.data() + indent);
	}

	// 1st line (code only)
	std::string_view code_line; // contains "    white = RGB([2, 5, 5], 255, 255)"
	// 2nd line
	std::size_t indent = 0; // distance from line start to "w"
	std::size_t spaces = 0; // distance from "w"        to first "~"
	std::size_t underlines = 0; // number of "~" characters
};

class code_underliner
{
public:
	// will produce 1+ usable next()
	code_underliner(std::string_view all_code, std::string_view code_to_underline);
	// will produce only 1 usable next()
	code_underliner(std::string_view all_code, const char* character_to_underline);

	underlined_code next() noexcept;

private:
	const char *const all_first;
	const char *const all_last;
	const char *const underline_first;
	const char *const underline_last;
	const char *      code_it;
	const char *const code_last;
};

std::string range_underline_to_string(
	std::string_view all_code,
	std::string_view code_to_underline);

}
