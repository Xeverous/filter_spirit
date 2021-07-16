#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/gui_settings.hpp>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

namespace fs::gui {

class settings_window : public imgui_window
{
public:
	settings_window()
	: imgui_window("Settings") {}

	      gui_settings& settings()       { return _settings; }
	const gui_settings& settings() const { return _settings; }

	void draw(Magnum::Platform::Application& application)
	{
		draw_window([this, &application]() { draw_impl(application); });
	}

private:
	void draw_impl(Magnum::Platform::Application& application);

	gui_settings _settings;
};

}
