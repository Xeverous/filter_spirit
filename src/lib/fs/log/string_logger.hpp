#pragma once

#include <fs/log/logger.hpp>

#include <string>

namespace fs::log
{

/**
 * @class string logger - stores input in a string
 */
class string_logger : public logger
{
public:
	void begin_message(severity /* s */) override
	{
	}

	void end_message() override
	{
	}

	void add_str(std::string_view text) override
	{
		buffer += text;
	}

	void add_char(char character) override
	{
		buffer += character;
	}

	void add_intmax(std::intmax_t number) override
	{
		buffer += std::to_string(number);
	}

	void add_uintmax(std::uintmax_t number) override
	{
		buffer += std::to_string(number);
	}

	      std::string& str()       { return buffer; }
	const std::string& str() const { return buffer; }

	void clear() { buffer.clear(); }

private:
	std::string buffer;
};

}
