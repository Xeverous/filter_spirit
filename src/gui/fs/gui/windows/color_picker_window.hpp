#pragma once

#include <fs/gui/imgui_window.hpp>

#include <Magnum/Math/Color.h>
#include <Magnum/Magnum.h>

namespace fs::gui {

class application;

class color_picker_window : public imgui_window
{
public:
	color_picker_window(application& app)
	: imgui_window("Color picker")
	, _application(app)
	{
	}

protected:
	void draw_contents() override;

private:
	application& _application;
	Magnum::Color4 _selected_color = Magnum::Color4::fromSrgbAlpha(0xffffffff);
};

}
