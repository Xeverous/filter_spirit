#include <fs/gui/windows/common_ui_settings_window.hpp>
#include <fs/gui/application.hpp>

#include <Magnum/GL/Renderer.h>

#include <imgui.h>

namespace fs::gui {

void common_ui_settings_window::draw_contents()
{
	if (ImGui::CollapsingHeader("Interface")) {
		if (ImGui::ColorEdit3("Background color", _clear_color.data()))
			Magnum::GL::Renderer::setClearColor(_clear_color);

		_application.theme_settings().draw_theme_selection_ui();
		_application.font_settings().draw_font_selection_ui();
	}

	if (ImGui::CollapsingHeader("Rendering")) {
#ifdef CORRADE_TARGET_ANDROID
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#else
		if (ImGui::SliderInt("min frame time (ms)", &_min_frame_time_ms, 0, 40))
			_application.setMinimalLoopPeriod(_min_frame_time_ms);
#endif

		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0 / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}

}
