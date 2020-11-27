#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/gui_logger.hpp>
#include <fs/log/buffer_logger.hpp>

#include <functional>

namespace fs::gui {

class application_log_window : public imgui_window
{
public:
	application_log_window(const fonting& f)
	: imgui_window("Application log")
	, _fonting(std::ref(f))
	{
	}

	log::logger& logger()
	{
		return _buffer_logger;
	}

protected:
	void draw_contents() override;

private:
	std::reference_wrapper<const fonting> _fonting;
	log::buffer_logger _buffer_logger;
	gui_logger _gui_logger;
};

}
