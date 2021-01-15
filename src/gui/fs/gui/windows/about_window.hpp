#pragma once

#include <fs/gui/imgui_window.hpp>

#include <string>

namespace fs::gui {

class about_window : public imgui_window
{
public:
	about_window();

protected:
	void draw_contents() override;
};

}
