#include <fs/gui/windows/common_ui_settings_window.hpp>

#include <imgui.h>

#include <Magnum/GL/Renderer.h>

namespace fs::gui {

void common_ui_settings_window::draw_contents()
{
	if (ImGui::CollapsingHeader("Interface")) {
		if (ImGui::ColorEdit3("Background color", _clear_color.data()))
			Magnum::GL::Renderer::setClearColor(_clear_color);

		_theming.draw_theme_selection_ui();

		ImGuiIO& io = ImGui::GetIO();
		ImFont* font_current = ImGui::GetFont();
		if (ImGui::BeginCombo("Application font", font_current->GetDebugName())) {
			for (int n = 0; n < io.Fonts->Fonts.Size; n++) {
				ImFont* font = io.Fonts->Fonts[n];
				ImGui::PushID(font);
				if (ImGui::Selectable(font->GetDebugName(), font == font_current))
					io.FontDefault = font;
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
	}

	if (ImGui::CollapsingHeader("Rendering")) {
#ifdef CORRADE_TARGET_ANDROID
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#else
		if (ImGui::SliderInt("min frame time (ms)", &_min_frame_time_ms, 0, 40))
			application.setMinimalLoopPeriod(_min_frame_time_ms);
#endif

		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0 / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}

}
