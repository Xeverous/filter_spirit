#pragma once

#include "fs/log/utility.hpp"

#include <string_view>

/**
 * how to use a logger:
 *
 * convenience syntax for whole messages in 1 statement:
 *
 *     logger.warning() << str << num << "text";
 *
 * manual input:
 *
 *     logger.begin_warning_message();
 *     logger.add(str)
 *     logger << num << "text";
 *     logger.end_message();
 */

namespace fs::log
{

class logger;

class logger_wrapper
{
public:
	~logger_wrapper();

	// implemented as member because non-member overloads of << can not take rvalues as parameter of type logger&
	template <typename T>
	logger_wrapper& operator<<(const T& val);

private:
	friend class logger;
	logger_wrapper(logger& logger) : logger(logger) {}

	logger& logger;
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

	logger_wrapper info()
	{
		begin_info_message();
		return logger_wrapper(*this);
	}

	logger_wrapper warning()
	{
		begin_warning_message();
		return logger_wrapper(*this);
	}

	logger_wrapper error()
	{
		begin_error_message();
		return logger_wrapper(*this);
	}

	virtual void begin_info_message   () = 0;
	virtual void begin_warning_message() = 0;
	virtual void begin_error_message  () = 0;
	virtual void end_message() = 0;

	virtual void add(std::string_view text) = 0;
	virtual void add(char character) = 0;
	virtual void add(int number) = 0;

	/**
	 * @details print error with detailed code information
	 * @param all_code range of all code, used to calculate line number (do not trim it)
	 * @param code_to_underline fragment of code to underline, must be a subview of @p all_code
	 * @param description anything accepted by logger, used as error description
	 *
	 * @details example:
	 *
	 *     line 12: error: expected object of type 'int', got 'array'
	 *     const white = RGB([2, 5, 5], 255, 255)
	 *                       ~~~~~~~~~
	 */
	template <typename... Printable>
	void error_with_underlined_code(
		std::string_view all_code,
		std::string_view code_to_underline,
		Printable&&... description);

	/**
	 * @brief print code with underlined part
	 * @param all_code all code to search for line breaks
	 * @param underlined_code code to underline, must be a subview of @p all_code, may span multiple lines
	 */
	void print_underlined_code(std::string_view all_code, std::string_view underlined_code);

	void internal_error(std::string_view description);
};

template <typename T>
logger_wrapper& logger_wrapper::operator<<(const T& val)
{
	logger << val;
	return *this;
}

template <typename T>
logger& operator<<(logger& logger, const T& val)
{
	logger.add(val); // if you get an error here with boost::iterator_range use fs::parser::to_string_view from parser/utility.hpp
	return logger;
}

// this needs to be outside class definition due to depency on operator<<
template <typename... Printable>
void logger::error_with_underlined_code(
	std::string_view all_code,
	std::string_view code_to_underline,
	Printable&&... description)
{
	((*this << "line " << count_lines(all_code.data(), code_to_underline.data())
		<< ": ") << ... << description) << '\n';
	print_underlined_code(all_code, code_to_underline);
}

}
