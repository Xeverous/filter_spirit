#pragma once

#include "log/logger.hpp"

#include <string>

namespace fs
{

/**
 * @class buffered logger - stores input in a buffer
 * untill explicitly asked to flush it out
 */
class buffered_logger : public logger
{
public:
	void begin_info_message   () override;
	void begin_warning_message() override;
	void begin_error_message  () override;
	void end_message() override;

	void add(std::string_view text) override;
	void add(char character) override;
	void add(int number) override;

	std::string flush_out();

private:
	std::string buffer;
};

}
