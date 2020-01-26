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

	void add(std::string_view /* text */) override {}
	void add(char /* character */) override {}
	void add(std::int64_t /* number */) override {}
	void add(std::uint64_t /* number */) override {}
};

}
