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

	void add(std::string_view text) override
	{
		buffer += text;
	}

	void add(char character) override
	{
		buffer += character;
	}

	void add(std::int64_t number) override
	{
		buffer += std::to_string(number);
	}

	void add(std::uint64_t number) override
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
