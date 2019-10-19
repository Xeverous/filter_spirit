#pragma once

#include <fs/lang/types.hpp>

#include <utility>
#include <iosfwd>

namespace fs::lang
{

struct action_set
{
	void override_border_color(color new_color)
	{
		border_color = new_color;
	}

	void override_text_color(color new_color)
	{
		text_color = new_color;
	}

	void override_background_color(color new_color)
	{
		background_color = new_color;
	}

	void override_font_size(font_size new_font_size)
	{
		font_size = new_font_size;
	}

	void override_alert_sound(alert_sound new_alert_sound)
	{
		alert_sound = std::move(new_alert_sound);
	}

	void override_play_default_drop_sound(bool enable)
	{
		disabled_drop_sound = !enable;
	}

	void override_minimap_icon(minimap_icon new_minimap_icon)
	{
		minimap_icon = new_minimap_icon;
	}

	void override_beam_effect(beam_effect new_beam_effect)
	{
		beam_effect = new_beam_effect;
	}

	void generate(std::ostream& output_stream) const;

	std::optional<color> border_color;
	std::optional<color> text_color;
	std::optional<color> background_color;
	std::optional<font_size> font_size;
	std::optional<alert_sound> alert_sound;
	bool disabled_drop_sound = false;
	std::optional<minimap_icon> minimap_icon;
	std::optional<beam_effect> beam_effect;
};

}
