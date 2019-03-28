#pragma once

#include <boost/range/iterator_range.hpp>

#include <string>
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

namespace fs
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

	logger_wrapper info();
	logger_wrapper warning();
	logger_wrapper error();

	virtual void begin_info_message   () = 0;
	virtual void begin_warning_message() = 0;
	virtual void begin_error_message  () = 0;
	virtual void end_message() = 0;

	virtual void add(std::string_view text) = 0;
	virtual void add(int number) = 0;
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
	logger.add(val);
	return logger;
}

inline
logger& operator<<(logger& logger, boost::iterator_range<std::string::const_iterator> range)
{
	// std::string iterators guuarantee continuous storage
	const auto first = range.begin();
	const auto last  = range.end();
	logger.add(std::string_view(&*first, last - first)); // use std::string_view(const char*, size_type) ctor overload
	return logger;
}

}
