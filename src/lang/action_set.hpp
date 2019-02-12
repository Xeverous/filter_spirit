#pragma once
#include "lang/types.hpp"

namespace fs::lang
{

struct action_set
{
	std::optional<bool> show;
	std::optional<color> border_color;
	std::optional<color> text_color;
	std::optional<color> background_color;
	std::optional<number> font_size;
	std::optional<alert_sound> sound;
	std::optional<std::monostate> disabled_drop_sound;
	std::optional<minimap_icon> icon;
	std::optional<beam_effect> effect;

	[[nodiscard]]
	action_set override_by(const action_set& other) const;
};

}
