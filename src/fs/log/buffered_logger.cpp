#include "fs/log/buffered_logger.hpp"

#include <iostream>
#include <utility>

namespace fs
{

void buffered_logger::begin_info_message()
{
	buffer += "INFO: ";
}

void buffered_logger::begin_warning_message()
{
	buffer += "WARN: ";
}

void buffered_logger::begin_error_message()
{
	buffer += "ERROR: ";
}

void buffered_logger::end_message()
{
	buffer += '\n';
}

void buffered_logger::add(std::string_view text)
{
	buffer += text;
}

void buffered_logger::add(char character)
{
	buffer += character;
}

void buffered_logger::add(int number)
{
	buffer += std::to_string(number);
}

std::string buffered_logger::flush_out()
{
	std::string log = std::move(buffer);
	buffer.clear();
	return log;
}

}
