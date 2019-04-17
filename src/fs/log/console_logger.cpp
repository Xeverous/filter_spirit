#include "fs/log/console_logger.hpp"

#include <iostream>

namespace fs
{

void console_logger::begin_info_message()
{
	std::cout << "INFO: ";
}

void console_logger::begin_warning_message()
{
	std::cout << "WARN: ";
}

void console_logger::begin_error_message()
{
	std::cout << "ERROR: ";
}

void console_logger::end_message()
{
	std::cout << '\n';
}

void console_logger::add(std::string_view text)
{
	std::cout << text;
}

void console_logger::add(char character)
{
	std::cout << character;
}

void console_logger::add(int number)
{
	std::cout << number;
}

}
