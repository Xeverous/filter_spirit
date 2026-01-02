#pragma once

#include <nukleus.hpp>

namespace fs::gui {

struct ui_state
{
	int window_width {};
	int window_height {};

	unsigned font_tex{};
	nk_draw_null_texture tex_null{};
	nk::font_atlas atlas = nk::font_atlas::init_default();

	float font_size = 18.0f;
	nk_font* fontin_regular {};
	nk_font* fontin_small_caps {};
	nk_font* monospace {};

	void regenerate_font_atlas();
};

}
