#pragma once

#include <fs/log/logger.hpp>
#include <fs/log/buffer_logger.hpp>
#include <fs/gui/windows/settings/fonting.hpp>

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

private:
	std::reference_wrapper<const fonting> _fonting; // plain reference type would make this class non-moveable
	fs::log::buffer_logger _logger;
};

}
