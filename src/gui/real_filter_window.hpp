#pragma once

#include "imgui_window.hpp"

#include <string>
#include <utility>

namespace fs::gui {

class real_filter_window : public imgui_window
{
public:
	real_filter_window(std::string path)
	: imgui_window(std::move(path))
	{
		open();
	}

protected:
	void draw_contents() override;

private:

};

}
