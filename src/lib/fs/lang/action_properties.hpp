#pragma once

namespace fs::lang {

enum class official_action_property
{
	set_text_color,
	set_border_color,
	set_background_color,
	set_font_size,
	play_effect,
	minimap_icon,
	play_alert_sound,
	play_alert_sound_positional,
	custom_alert_sound,
	custom_alert_sound_optional,
	disable_drop_sound,
	enable_drop_sound,
	disable_drop_sound_if_alert_sound,
	enable_drop_sound_if_alert_sound
};

} // namespace fs::lang
