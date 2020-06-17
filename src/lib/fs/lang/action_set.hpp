#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/position_tag.hpp>

#include <utility>
#include <tuple>
#include <iosfwd>

namespace fs::lang
{

// origins should point to the strings which create actions, eg:
// SetTextColor 0 0 0 255
// ~~~~~~~~~~~~

struct color
{
	color(integer r, integer g, integer b, std::optional<integer> a = std::nullopt)
	: r(r), g(g), b(b), a(a) {}

	integer r;
	integer g;
	integer b;
	std::optional<integer> a;
};

inline bool operator==(color lhs, color rhs) noexcept
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.a) == std::tie(rhs.r, rhs.g, rhs.b, rhs.a);
}
inline bool operator!=(color lhs, color rhs) noexcept { return !(lhs == rhs); }


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
	integer size;
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

struct builtin_alert_sound_id
{
	explicit builtin_alert_sound_id(integer id)
	: id(id) {}

	explicit builtin_alert_sound_id(shaper_voice_line svl)
	: id(svl) {}

	std::variant<integer, shaper_voice_line> id;
};

inline bool operator==(builtin_alert_sound_id lhs, builtin_alert_sound_id rhs)
{
	return lhs.id == rhs.id;
}

inline bool operator!=(builtin_alert_sound_id lhs, builtin_alert_sound_id rhs)
{
	return !(lhs == rhs);
}

struct builtin_alert_sound
{
	explicit builtin_alert_sound(bool is_positional, builtin_alert_sound_id sound_id, std::optional<integer> volume = std::nullopt)
	: is_positional(is_positional), sound_id(sound_id), volume(volume) {}

	bool is_positional;
	builtin_alert_sound_id sound_id;
	std::optional<integer> volume;
};

inline bool operator==(builtin_alert_sound lhs, builtin_alert_sound rhs) noexcept
{
	return std::tie(lhs.is_positional, lhs.sound_id, lhs.volume)
		== std::tie(rhs.is_positional, rhs.sound_id, rhs.volume);
}
inline bool operator!=(builtin_alert_sound lhs, builtin_alert_sound rhs) noexcept { return !(lhs == rhs); }

struct custom_alert_sound
{
	explicit custom_alert_sound(string path)
	: path(std::move(path)) {}

	string path;
};

inline bool operator==(const custom_alert_sound& lhs, const custom_alert_sound& rhs) noexcept { return lhs.path == rhs.path; }
inline bool operator!=(const custom_alert_sound& lhs, const custom_alert_sound& rhs) noexcept { return !(lhs == rhs); }

struct alert_sound
{
	explicit alert_sound(builtin_alert_sound sound)
	: sound(sound) {}

	explicit alert_sound(custom_alert_sound sound)
	: sound(std::move(sound)) {}

	std::variant<builtin_alert_sound, custom_alert_sound> sound;
};

inline bool operator==(const alert_sound& lhs, const alert_sound& rhs) noexcept { return lhs.sound == rhs.sound; }
inline bool operator!=(const alert_sound& lhs, const alert_sound& rhs) noexcept { return !(lhs == rhs); }

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
struct disable_drop_sound_action
{
	position_tag origin;
};

inline bool operator==(disable_drop_sound_action /* lhs */, disable_drop_sound_action /* rhs */)
{
	return true;
}

inline bool operator!=(disable_drop_sound_action lhs, disable_drop_sound_action rhs)
{
	return !(lhs == rhs);
}

struct minimap_icon
{
	minimap_icon(integer size, suit color, shape shape_)
	: size(size), color(color), shape_(shape_) {}

	integer size;
	suit color;
	shape shape_;
};

inline bool operator==(minimap_icon lhs, minimap_icon rhs) noexcept
{
	return std::tie(lhs.size, lhs.color, lhs.shape_) == std::tie(rhs.size, rhs.color, rhs.shape_);
}
inline bool operator!=(minimap_icon lhs, minimap_icon rhs) noexcept { return !(lhs == rhs); }

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

struct play_effect
{
	play_effect(suit s, bool is_temporary)
	: color(s), is_temporary(is_temporary) {}

	suit color;
	bool is_temporary;
};

inline bool operator==(play_effect lhs, play_effect rhs) noexcept
{
	return std::tie(lhs.color, lhs.is_temporary) == std::tie(rhs.color, rhs.is_temporary);
}
inline bool operator!=(play_effect lhs, play_effect rhs) noexcept { return !(lhs == rhs); }

struct play_effect_action
{
	play_effect beam;
	position_tag origin;
};

inline bool operator==(play_effect_action lhs, play_effect_action rhs)
{
	return lhs.beam == rhs.beam;
}

inline bool operator!=(play_effect_action lhs, play_effect_action rhs)
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
	std::optional<alert_sound_action> play_alert_sound;
	std::optional<disable_drop_sound_action> disable_drop_sound;
	std::optional<minimap_icon_action> minimap_icon;
	std::optional<play_effect_action> play_effect;
};

bool operator==(const action_set& lhs, const action_set& rhs);
bool operator!=(const action_set& lhs, const action_set& rhs);

}
