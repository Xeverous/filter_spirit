#include <fs/gui/windows/settings_window.hpp>

#include <Magnum/GL/Renderer.h>

#include <imgui.h>

namespace fs::gui {

void settings_window::draw_impl(Magnum::Platform::Application& application)
{
	if (ImGui::CollapsingHeader("Interface", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::ColorEdit3("Background color", _settings.clear_color.data()))
			Magnum::GL::Renderer::setClearColor(_settings.clear_color);

		_settings.theming.draw_theme_selection_ui();
		_settings.fonting.draw_font_selection_ui();
	}

	if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
#ifndef CORRADE_TARGET_EMSCRIPTEN
		if (ImGui::SliderInt("min frame time (ms)", &_settings.min_frame_time_ms, 0, 40))
			application.setMinimalLoopPeriod(_settings.min_frame_time_ms);
#endif

		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0 / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	if (ImGui::CollapsingHeader("Networking", ImGuiTreeNodeFlags_DefaultOpen)) {
		_settings.networking.draw_interface();
	}
}

}
