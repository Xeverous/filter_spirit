#pragma once

#include <fs/log/buffer_logger.hpp>

#include <cstddef>

namespace fs::gui {

class font_settings;

class gui_logger
{
public:
	void draw(const font_settings& fonting, log::buffer_logger& logger);

private:
	std::size_t _last_log_size = 0;
};

}
