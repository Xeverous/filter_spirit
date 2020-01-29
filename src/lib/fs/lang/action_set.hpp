#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/position_tag.hpp>

#include <utility>
#include <iosfwd>

namespace fs::lang
{

// origins should point to the strings which create actions, eg:
// SetTextColor 0 0 0 255
// ~~~~~~~~~~~~

struct color_action
{
	color c;
	position_tag origin;
};

inline bool operator==(color_action lhs, color_action rhs)
{
	return lhs.c == rhs.c;
}

inline bool operator!=(color_action lhs, color_action rhs)
{
	return !(lhs == rhs);
}

struct font_size_action
{
	font_size size;
	position_tag origin;
};

inline bool operator==(font_size_action lhs, font_size_action rhs)
{
	return lhs.size == rhs.size;
}

inline bool operator!=(font_size_action lhs, font_size_action rhs)
{
	return !(lhs == rhs);
}

struct alert_sound_action
{
	alert_sound alert;
	position_tag origin;
};

inline bool operator==(alert_sound_action lhs, alert_sound_action rhs)
{
	return lhs.alert == rhs.alert;
}

inline bool operator!=(alert_sound_action lhs, alert_sound_action rhs)
{
	return !(lhs == rhs);
}

// this action has no state - it either exists (we disabled default drop sound)
// or does not exist (empty optional, no emmiting of this action in the filter)
struct disable_default_drop_sound_action
{
	position_tag origin;
};

inline bool operator==(disable_default_drop_sound_action /* lhs */, disable_default_drop_sound_action /* rhs */)
{
	return true;
}

inline bool operator!=(disable_default_drop_sound_action lhs, disable_default_drop_sound_action rhs)
{
	return !(lhs == rhs);
}

struct minimap_icon_action
{
	minimap_icon icon;
	position_tag origin;
};

inline bool operator==(minimap_icon_action lhs, minimap_icon_action rhs)
{
	return lhs.icon == rhs.icon;
}

inline bool operator!=(minimap_icon_action lhs, minimap_icon_action rhs)
{
	return !(lhs == rhs);
}

struct beam_effect_action
{
	beam_effect beam;
	position_tag origin;
};

inline bool operator==(beam_effect_action lhs, beam_effect_action rhs)
{
	return lhs.beam == rhs.beam;
}

inline bool operator!=(beam_effect_action lhs, beam_effect_action rhs)
{
	return !(lhs == rhs);
}

/**
 * @class a type describing all of the given filter block actions
 *
 * all values are optional which is self-explanatory - no actions are required
 *
 * this type assumes all values (color, sizes, strings) are already verified and correct
 */
struct action_set
{
	void override_with(action_set&& other);

	void generate(std::ostream& output_stream) const;

	std::optional<color_action> set_border_color;
	std::optional<color_action> set_text_color;
	std::optional<color_action> set_background_color;
	std::optional<font_size_action> set_font_size;
	std::optional<alert_sound_action> set_alert_sound;
	std::optional<disable_default_drop_sound_action> disable_default_drop_sound;
	std::optional<minimap_icon_action> set_minimap_icon;
	std::optional<beam_effect_action> set_beam_effect;
};

bool operator==(const action_set& lhs, const action_set& rhs);
bool operator!=(const action_set& lhs, const action_set& rhs);

}
