#pragma once

#include "fs/log/logger.hpp"

namespace fs::log
{

/**
 * @class console logger (writes to stdout)
 */
class console_logger : public logger
{
public:
	void begin_info_message   () override;
	void begin_warning_message() override;
	void begin_error_message  () override;
	void end_message() override;

	void add(std::string_view text) override;
	void add(char character) override;
	void add(int number) override;
};

}
