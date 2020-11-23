#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/auxiliary/color_convert.hpp>

#include <Magnum/Math/Color.h>

namespace fs::gui {

class application;

class settings_window : public imgui_window
{
public:
	settings_window(application& app)
	: imgui_window("Settings")
	, _application(app)
	{
	}

	int min_frame_time_ms() const
	{
		return _min_frame_time_ms;
	}

protected:
	void draw_contents() override;

private:
	application& _application;
	Magnum::Color3 _clear_color = aux::from_rgb(0x1f1f1f);
	int _min_frame_time_ms = 16; // 16ms per frame, roughly 60 FPS
};

}
