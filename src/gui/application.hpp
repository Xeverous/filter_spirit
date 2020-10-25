#pragma once

#include "color_picker_window.hpp"
#include "single_item_preview_window.hpp"
#include "common_ui_settings_window.hpp"
#include "version_info_window.hpp"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

namespace fs::gui {

class application: public Magnum::Platform::Application
{
public:
	explicit application(const Arguments& arguments);

	void drawEvent() override;

	void viewportEvent(ViewportEvent& event) override;

	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;

	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void mouseScrollEvent(MouseScrollEvent& event) override;
	void textInputEvent(TextInputEvent& event) override;

private:
	void draw_main_menu_bar();

	Magnum::ImGuiIntegration::Context _imgui{Magnum::NoCreate};

	ImFont* _fontin_regular = nullptr;
	ImFont* _fontin_small_caps = nullptr;

	bool _show_demo_window = false;
	bool _force_focus_demo_window = false;

	color_picker_window _color_picker;
	single_item_preview_window _single_item_preview;
	common_ui_settings_window _common_ui_settings;
	version_info_window _version_info;
};

}
