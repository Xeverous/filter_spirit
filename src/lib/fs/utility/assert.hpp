#pragma once

#include <boost/assert.hpp>

#ifdef BOOST_ENABLE_ASSERT_HANDLER
#include <stdexcept>
#include <string>

namespace fs
{
	class assertion_failure: public std::logic_error
	{
	public:
		assertion_failure(
			const char* expr,
			const char* function,
			const char* file,
			long line)
		: std::logic_error(std::string(file) + ":" + std::to_string(line) + ": assertion failure"
			"\nIn function '" + function + "': " + expr)
		{
		}

		assertion_failure(
			const char* expr,
			const char* msg,
			const char* function,
			const char* file,
			long line)
		: std::logic_error(std::string(file) + ":" + std::to_string(line) + ": assertion failure: "
			+ msg + "\nIn function '" + function + "': " + expr)
		{
		}
	};
}

namespace boost
{
	inline void assertion_failed(
		const char* expr,
		const char* function,
		const char* file,
		long line)
	{
		throw fs::assertion_failure(expr, function, file, line);
	}

	inline void assertion_failed_msg(
		const char* expr,
		const char* msg,
		const char* function,
		const char* file,
		long line)
	{
		throw fs::assertion_failure(expr, msg, function, file, line);
	}
}

#endif
