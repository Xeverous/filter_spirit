#pragma once

#include <fs/gui/imgui_window.hpp>

#include <string>

namespace fs::gui {

class version_info_window : public imgui_window
{
public:
	version_info_window();

protected:
	void draw_contents() override;

private:
	const std::string _fs_version;
	const std::string _boost_version;
	const std::string _opengl_version;
	const std::string _sdl_version;
	const std::string _cpp_version;
};

}
