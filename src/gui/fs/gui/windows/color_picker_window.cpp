#include <fs/gui/windows/color_picker_window.hpp>
#include <fs/gui/ui_utils.hpp>

#include <imgui.h>

#include <Magnum/Math/Packing.h>

#include <string>

namespace {

void button_click_to_copy(const char* text)
{
	if (ImGui::Button(text))
		ImGui::SetClipboardText(text);
}

}

namespace fs::gui {

void color_picker_window::draw_contents()
{
	const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar;
	ImVec4 button_col = {
		_selected_color.x(),
		_selected_color.y(),
		_selected_color.z(),
		_selected_color.w()
	};
	ImGui::ColorButton("", button_col, flags, ImVec2(100, 100));
	ImGui::SameLine();
	ImGui::Text("click to copy\nthese strings");
	ImGui::SameLine();
	ImGui::BeginGroup();

	const auto rgba = to_rgba(_selected_color);
	const auto rgba_str = std::to_string(rgba.r()) + " " + std::to_string(rgba.g()) + " " + std::to_string(rgba.b()) + " " + std::to_string(rgba.a());
	button_click_to_copy(rgba_str.c_str());
	button_click_to_copy(("SetBorderColor " + rgba_str).c_str());
	button_click_to_copy(("SetTextColor " + rgba_str).c_str());
	button_click_to_copy(("SetBackgroundColor " + rgba_str).c_str());

	ImGui::EndGroup();
	ImGui::ColorPicker4("", _selected_color.data(), flags);
}

}
