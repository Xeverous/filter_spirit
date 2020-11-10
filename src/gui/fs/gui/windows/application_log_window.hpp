#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/gui_logger.hpp>

namespace fs::gui {

class application_log_window : public imgui_window
{
public:
	application_log_window(fonting& f)
	: imgui_window("Application log")
	, _logger(f)
	{
	}

	log::logger& logger()
	{
		return _logger.logger();
	}

protected:
	void draw_contents() override;

private:
	gui_logger _logger;
};

}
