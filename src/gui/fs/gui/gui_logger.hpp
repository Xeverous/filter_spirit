#pragma once

#include <fs/gui/settings/fonting.hpp>
#include <fs/log/buffer_logger.hpp>

#include <functional>

namespace fs::gui {

class gui_logger
{
public:
	gui_logger(const fonting& f)
	: _fonting(std::cref(f))
	{
	}

	auto& logger()
	{
		return _logger;
	}

	const auto& logger() const
	{
		return _logger;
	}

	void draw();
	void clear_logs();

private:
	std::reference_wrapper<const fonting> _fonting; // plain reference type would make this class non-moveable
	log::buffer_logger _logger;
	std::size_t _last_log_size = 0;
};

}
