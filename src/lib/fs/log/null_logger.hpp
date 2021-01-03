#pragma once

#include <fs/log/logger.hpp>

namespace fs::log
{

/**
 * @class null logger (ignores any input)
 *
 * used for unit testing where logging has no impact
 */
class null_logger : public logger
{
public:
	void begin_message(severity /* s */) override {}
	void end_message() override {}

	void add_str(std::string_view /* text */) override {}
	void add_char(char /* character */) override {}
	void add_intmax(std::intmax_t /* number */) override {}
	void add_uintmax(std::uintmax_t /* number */) override {}
};

}
