#include <fs/gui/windows/single_item_preview_window.hpp>

#include <imgui.h>

namespace fs::gui {

void single_item_preview_window::draw_impl()
{
	ImGui::InputText("", _item_name_buf.data(), _item_name_buf.size());
}

}
