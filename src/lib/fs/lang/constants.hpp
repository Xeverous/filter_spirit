#pragma once

namespace fs::lang::constants
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

	constexpr auto min_memory_strands = 0;
	constexpr auto max_memory_strands = 100;

	// ---- actions ----

	constexpr auto default_set_text_color_opacity       = 255;
	constexpr auto default_set_border_color_opacity     = 255;
	constexpr auto default_set_background_color_opacity = 240;

	constexpr auto ruthless_min_set_text_color_opacity  =  80;

	// these are not strictly enforced by the filter
	// but the game client clamps values in this range
	constexpr auto min_filter_font_size = 1;
	constexpr auto max_filter_font_size = 45;

	constexpr auto min_filter_sound_id = 1;
	constexpr auto max_filter_sound_id = 16;

	constexpr auto min_filter_volume = 0;
	constexpr auto max_filter_volume = 300;

	constexpr auto default_play_alert_sound_volume   =  50; // PlayAlertSound, PlayAlertSoundPositional
	constexpr auto default_custom_alert_sound_volume = 100; // CustomAlertSound, CustomAlertSoundOptional

	constexpr auto min_item_level = 0;
	constexpr auto max_item_level = 100;
}
