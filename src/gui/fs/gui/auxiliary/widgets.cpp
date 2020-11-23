#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/auxiliary/raii.hpp>

#include <imgui.h>

namespace fs::gui::aux {

void on_hover_text_tooltip(std::string_view text)
{
	if (ImGui::IsItemHovered())
	{
		scoped_tooltip _;
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(text.data(), text.data() + text.size());
		ImGui::PopTextWrapPos();
	}
}

}
