#pragma once

#include <fs/utility/string_helpers.hpp>

#include <string_view>
#include <cstdint>
#include <utility>

namespace fs::log
{

enum class severity { info, warning, error };

class logger;

/**
 * @brief a type representing a single log message
 *
 * construtions begins the message, destruction ends it
 */
class message_stream
{
public:
	message_stream(severity s, logger& l);
	~message_stream();

	friend void swap(message_stream& lhs, message_stream& rhs) noexcept;

	message_stream(const message_stream&) = delete;
	message_stream& operator=(message_stream) = delete;

	message_stream(message_stream&& other) noexcept
	: message_stream()
	{
		swap(*this, other);
	}

	message_stream& operator=(message_stream&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	// implemented as member because non-member overloads of << can not take rvalues as parameter of type logger&
	// said differently, log.error() returns a message_stream rvalue and we want to support immediate << on it
	template <typename T>
	message_stream& operator<<(const T& val);

	friend message_stream& operator<<(message_stream& stream, severity s);

	void print_line_number(int line_number);

	template <typename... Printable>
	void print_line_number_with_description(
		int line_number,
		Printable&&... printable);

	/**
	 * @details print error with detailed code information
	 * @param all_code range of all code, used to calculate line number (do not trim it)
	 * @param code_to_underline fragment of code to underline, must be a subview of @p all_code
	 * @param description anything accepted by logger, used as error description
	 *
	 * @details example:
	 *
	 *     line 12: expected object of type 'int', got 'array'
	 *     const white = RGB([2, 5, 5], 255, 255)
	 *                       ~~~~~~~~~
	 */
	template <typename... Printable>
	void print_line_number_with_description_and_underlined_code(
		std::string_view all_code,
		std::string_view code_to_underline,
		Printable&&... description);
	/**
	 * @details print error with detailed code information
	 * @param all_code range of all code, used to calculate line number (do not trim it)
	 * @param point character to point out, must be in range of @p all_code, may point at line break
	 * @param description anything accepted by logger, used as error description
	 *
	 * @details example:
	 *
	 *     line 12: expected '}' here
	 *     {
	 *     ^
	 */
	template <typename... Printable>
	void print_line_number_with_description_and_pointed_code(
		std::string_view all_code,
		const char* point,
		Printable&&... description);
	/**
	 * @brief print code with underlined part
	 * @param all_code all code to search for line breaks
	 * @param code_to_underline code to underline, must be a subview of @p all_code, may span multiple lines
	 * @details example:
	 *
	 *     const white = RGB([2, 5, 5], 255, 255)
	 *                       ~~~~~~~~~
	 */
	void print_underlined_code(std::string_view all_code, std::string_view code_to_underline);
	void print_underlined_code(utility::underlined_code uc, char underline_char = '~');
	/**
	 * @brief print code with a pointed character
	 * @param all_code all code to search for line breaks
	 * @param character_to_underline character to point out, must be in range of @p all_code, may point at line break
	 * @details example:
	 *
	 *     }
	 *     ^
	 */
	void print_pointed_code(std::string_view all_code, const char* character_to_underline);

private:
	message_stream()
	: _logger(nullptr) {}

	logger* _logger;
};

/**
 * @class logger interface for implementing program messages
 *
 * @details sample implementations would be a console logger (stdout), GUI logger or cross-language API
 */
class logger
{
public:
	virtual ~logger() = default;

	[[nodiscard]] message_stream message(severity s)
	{
		return message_stream(s, *this);
	}

	[[nodiscard]] message_stream info()
	{
		return message(severity::info);
	}

	[[nodiscard]] message_stream warning()
	{
		return message(severity::warning);
	}

	[[nodiscard]] message_stream error()
	{
		return message(severity::error);
	}

	virtual void begin_logging() {}
	virtual void end_logging() {}

	virtual void add_str(std::string_view text) = 0;
	virtual void add_char(char character) = 0;
	virtual void add_intmax(std::intmax_t number) = 0;
	virtual void add_uintmax(std::uintmax_t number) = 0;

	void add(std::string_view text) { add_str(text); }
	void add(char character) { add_char(character); }

	void add(short number)     { add_intmax(number); }
	void add(int number)       { add_intmax(number); }
	void add(long number)      { add_intmax(number); }
	void add(long long number) { add_intmax(number); }

	void add(unsigned short number)     { add_uintmax(number); }
	void add(unsigned int number)       { add_uintmax(number); }
	void add(unsigned long number)      { add_uintmax(number); }
	void add(unsigned long long number) { add_uintmax(number); }

protected:
	friend class message_stream;
	friend message_stream& operator<<(message_stream& stream, severity s);

	virtual void begin_message(severity s) = 0;
	virtual void end_message() = 0;
};

template <typename T>
message_stream& message_stream::operator<<(const T& val)
{
	if (_logger)
		_logger->add(val);

	return *this;
}

// this needs to be outside class definition due to dependency on operator<<
template <typename... Printable>
void message_stream::print_line_number_with_description(
	int line_number,
	Printable&&... printable)
{
	print_line_number(line_number);
	(*this << ... << std::forward<Printable>(printable)) << '\n';
}

template <typename... Printable>
void message_stream::print_line_number_with_description_and_underlined_code(
	std::string_view all_code,
	std::string_view code_to_underline,
	Printable&&... description)
{
	print_line_number_with_description(
		utility::count_lines(all_code.data(), code_to_underline.data()),
		std::forward<Printable>(description)...);
	print_underlined_code(all_code, code_to_underline);
}

template <typename... Printable>
void message_stream::print_line_number_with_description_and_pointed_code(
	std::string_view all_code,
	const char* point,
	Printable&&... description)
{
	print_line_number_with_description(
		utility::count_lines(all_code.data(), point),
		std::forward<Printable>(description)...);
	print_pointed_code(all_code, point);
}

namespace strings
{

constexpr auto note = "note: ";
constexpr auto warning = "warning: ";
constexpr auto error = "error: ";

constexpr auto internal_compiler_error = "internal compiler error: ";
constexpr auto request_bug_report = "Please report a bug with attached minimal filter source that reproduces it.\n";

}

}
