#pragma once

#include <fs/gui/imgui_window.hpp>

#include <Magnum/Math/Color.h>
#include <Magnum/Magnum.h>

namespace fs::gui {

class font_settings;

class color_picker_window : public imgui_window
{
public:
	color_picker_window()
	: imgui_window("Color picker") {}

	void draw(const font_settings& fonting);

private:
	void draw_impl(const font_settings& fonting);

	Magnum::Color4 _selected_color = Magnum::Color4::fromSrgbAlpha(0xffffffff);
};

}
