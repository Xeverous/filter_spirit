#pragma once

#include <fs/log/logger.hpp>
#include <fs/log/buffer_logger.hpp>
#include <fs/gui/settings/fonting.hpp>

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

	void draw(float lines_of_text = 8.0f); // use 0 to occupy whole space

private:
	std::reference_wrapper<const fonting> _fonting; // plain reference type would make this class non-moveable
	fs::log::buffer_logger _logger;
};

}
