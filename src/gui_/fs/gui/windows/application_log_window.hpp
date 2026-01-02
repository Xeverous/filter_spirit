#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/gui_logger.hpp>
#include <fs/log/buffer_logger.hpp>

namespace fs::gui {

class application_log_window : public imgui_window
{
public:
	application_log_window()
	: imgui_window("Application log") {}

	log::logger& logger()
	{
		return _buffer_logger;
	}

	void draw(const font_settings& fonting)
	{
		draw_window([this, &fonting]() {
			ImGui::TextUnformatted(
				"Here you can find logs for the entire application.\n"
				"If some errors appear here, something must went horribly wrong.");
			_gui_logger.draw(fonting, _buffer_logger);
		});
	}

private:
	log::buffer_logger _buffer_logger;
	gui_logger _gui_logger;
};

}
