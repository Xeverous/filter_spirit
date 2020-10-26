#pragma once

#include "imgui_window.hpp"

#include <array>

namespace fs::gui {

class single_item_preview_window : public imgui_window
{
public:
	single_item_preview_window()
	: imgui_window("Single item preview")
	{
	}

	void draw_contents() override;

private:
	std::array<char, 64> _item_name_buf = { "Item Preview" };
};

}
