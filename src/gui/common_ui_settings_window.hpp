#pragma once

#include "imgui_window.hpp"
#include "theming.hpp"
#include "ui_utils.hpp"

#include <Magnum/Math/Color.h>
#include <Magnum/Magnum.h>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

namespace fs::gui {

class common_ui_settings_window : public imgui_window
{

public:
	common_ui_settings_window(Magnum::Platform::Application& application)
	: imgui_window("Common UI settings")
	, application(application)
	{
	}

	int min_frame_time_ms() const
	{
		return _min_frame_time_ms;
	}

protected:
	void draw_contents() override;

private:
	Magnum::Platform::Application& application;
	theming _theming;
	Magnum::Color3 _clear_color = from_rgb(0x1f1f1f);
	int _min_frame_time_ms = 16; // 16ms per frame, roughly 60 FPS
};

}
