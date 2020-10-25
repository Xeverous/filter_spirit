#pragma once

#include "imgui_window.hpp"

namespace fs::gui {

class version_info_window : public imgui_window
{
public:
	version_info_window()
	: imgui_window("Version information")
	{
	}

protected:
	void draw_contents() override;
};

}
