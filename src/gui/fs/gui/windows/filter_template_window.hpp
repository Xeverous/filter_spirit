#pragma once

#include <fs/gui/imgui_window.hpp>

#include <string>
#include <utility>

namespace fs::gui {

class filter_template_window : public imgui_window
{
public:
	filter_template_window(std::string path)
	: imgui_window(std::move(path))
	{
		open();
	}

protected:
	void draw_contents() override;

private:

};

}
