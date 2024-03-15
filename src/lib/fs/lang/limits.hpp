#pragma once

namespace fs::lang::limits
{
	// ---- conditions ----

	constexpr auto min_item_width   = 1;
	constexpr auto max_item_width   = 2;

	constexpr auto min_item_height  = 1;
	constexpr auto max_item_height  = 4;

	constexpr auto min_item_sockets = 0;
	constexpr auto max_item_sockets = 6;

	constexpr auto min_item_gem_level = 1;
	constexpr auto max_item_gem_level = 21;

	constexpr auto min_item_gem_quality = 0;
	constexpr auto max_item_gem_quality = 23;

	constexpr auto max_item_influences = 6;
	constexpr auto max_filter_influences = 6;

	// ---- actions ----

	constexpr auto ruthless_min_set_text_color_alpha = 80;

	// these are not strictly enforced by the filter
	// but the game client clamps values in this range
	constexpr auto min_filter_font_size = 18;
	constexpr auto max_filter_font_size = 45;

	constexpr auto min_filter_sound_id = 1;
	constexpr auto max_filter_sound_id = 16;

	constexpr auto min_filter_volume = 0;
	constexpr auto max_filter_volume = 300;

	constexpr auto min_item_level = 0;
	constexpr auto max_item_level = 100;

	constexpr auto default_filter_opacity = 240;
}
