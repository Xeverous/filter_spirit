#pragma once

#include <fs/gui/auxiliary/color_convert.hpp>
#include <fs/gui/settings/font_settings.hpp>
#include <fs/gui/settings/theme_settings.hpp>
#include <fs/gui/settings/network_settings.hpp>

#include <Magnum/Math/Color.h>

namespace fs::gui {

struct gui_settings
{
	font_settings fonting;
	theme_settings theming;
	network_settings networking;
	Magnum::Color3 clear_color = aux::from_rgb(0x1f1f1f);
	int min_frame_time_ms = 16; // 16ms per frame, roughly 60 FPS
};

}
