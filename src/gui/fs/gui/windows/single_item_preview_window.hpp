#pragma once

#include <fs/gui/imgui_window.hpp>

#include <array>

namespace fs::gui {

class single_item_preview_window : public imgui_window
{
public:
	single_item_preview_window()
	: imgui_window("Single item preview")
	{
	}

	void draw()
	{
		draw_window([this]() { draw_impl(); });
	}

private:
	void draw_impl();

	std::array<char, 64> _item_name_buf = { "Item Preview" };
};

}
