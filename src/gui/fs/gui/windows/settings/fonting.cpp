#include <fs/gui/windows/settings/fonting.hpp>

#include <imgui.h>

namespace fs::gui {

void fonting::draw_font_selection_ui()
{
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

}
