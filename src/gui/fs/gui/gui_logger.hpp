#pragma once

#include <fs/gui/settings/fonting.hpp>
#include <fs/log/buffer_logger.hpp>

#include <functional>

namespace fs::gui {

class gui_logger
{
public:
	void draw(const fonting& f, log::buffer_logger& logger);

private:
	std::size_t _last_log_size = 0;
};

}
