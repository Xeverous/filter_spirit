#pragma once

#include <fs/gui/imgui_window.hpp>

#include <string>

namespace fs::gui {

class about_window : public imgui_window
{
public:
	about_window()
	: imgui_window("About Filter Spirit") {}

	void draw()
	{
		draw_window([]() {
			ImGui::TextWrapped(
R"(Released under the terms of GNU General Public License 3.0.
Included fonts may be licensed differently - see their respective license files.

Filter Spirit is maintained by Xeverous
view documentation, source code and report bugs/feature requests/questions on github.com/Xeverous/filter_spirit
contact: /u/Xeverous on reddit, Xeverous#2151 on Discord, pathofexile.com/account/view-profile/Xeverous)");
		});
	}
};

}
