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

	void add_str(std::string_view text) override
	{
		std::cout << text;
	}

	void add_char(char character) override
	{
		std::cout << character;
	}

	void add_intmax(std::intmax_t number) override
	{
		std::cout << number;
	}

	void add_uintmax(std::uintmax_t number) override
	{
		std::cout << number;
	}
};

}
