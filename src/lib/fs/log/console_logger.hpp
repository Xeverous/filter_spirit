#pragma once

#include <fs/log/logger.hpp>

#include <iostream>

namespace fs::log
{

/**
 * @class console logger (writes to stdout)
 */
class console_logger : public logger
{
public:
	void begin_message(severity s) override
	{
		if (s == severity::info)
			std::cout << "INFO: ";
		else if (s == severity::warning)
			std::cout << "WARN: ";
		else if (s == severity::error)
			std::cout << "ERROR: ";
	}

	void end_message() override
	{
		std::cout << std::flush;
	}

	void add(std::string_view text) override
	{
		std::cout << text;
	}

	void add(char character) override
	{
		std::cout << character;
	}

	void add(std::int64_t number) override
	{
		std::cout << number;
	}

	void add(std::uint64_t number) override
	{
		std::cout << number;
	}
};

}
