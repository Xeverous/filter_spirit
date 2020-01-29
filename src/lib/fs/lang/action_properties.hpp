#pragma once

namespace fs::lang
{

enum class unary_action_type
{
	set_border_color,
	set_text_color,
	set_background_color,
	set_font_size,
	set_alert_sound,
	play_default_drop_sound,
	set_minimap_icon,
	set_beam
};

enum class color_action_type
{
	set_border_color,
	set_text_color,
	set_background_color
};

} // namespace fs::lang
