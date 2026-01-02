#include <fs/gui/windows/color_picker_window.hpp>
#include <fs/gui/settings/font_settings.hpp>
#include <fs/gui/auxiliary/color_convert.hpp>
#include <imgui.h>

#include <string>

namespace {

void button_click_to_copy(const char* text)
{
	if (ImGui::Button(text))
		ImGui::SetClipboardText(text);
}

}

namespace fs::gui {

void color_picker_window::draw(const font_settings& fonting)
{
	draw_window([this, &fonting](){ draw_impl(fonting); });
}

void color_picker_window::draw_impl(const font_settings& fonting)
{
	const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar;
	const ImVec4 button_color = {
		_selected_color.x(),
		_selected_color.y(),
		_selected_color.z(),
		_selected_color.w()
	};
	const auto button_size = fonting.monospaced_font_size() * 4; // * 4 because we draw 4 lines of text
	ImGui::ColorButton("", button_color, flags, ImVec2(button_size, button_size));
	ImGui::SameLine();
	ImGui::TextUnformatted("click to copy\nthese strings");
	ImGui::SameLine();

	{
		auto _ = fonting.scoped_monospaced_font();
		ImGui::BeginGroup();

		const auto rgba = aux::to_rgba(_selected_color);
		const auto rgba_str = std::to_string(rgba.r()) + " " + std::to_string(rgba.g()) + " " + std::to_string(rgba.b()) + " " + std::to_string(rgba.a());
		button_click_to_copy(rgba_str.c_str());
		button_click_to_copy(("SetBorderColor " + rgba_str).c_str());
		button_click_to_copy(("SetTextColor " + rgba_str).c_str());
		button_click_to_copy(("SetBackgroundColor " + rgba_str).c_str());

		ImGui::EndGroup();
	}

	ImGui::ColorPicker4("", _selected_color.data(), flags);
}

}
