#include "single_item_preview_window.hpp"

#include <imgui.h>

namespace fs::gui {

void single_item_preview_window::draw_contents()
{
	ImGui::InputText("Item Preview", _item_name_buf.data(), _item_name_buf.size());
}

}
