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

// TODO remove unneded ctors and types

struct color
{
	color(int r, int g, int b)
	: r(r), g(g), b(b) {}

	color(int r, int g, int b, int a)
	: r(r), g(g), b(b), a(a) {}

	color(integer r, integer g, integer b)
	: r(r.value), g(g.value), b(b.value) {}

	color(integer r, integer g, integer b, integer a)
	: r(r.value), g(g.value), b(b.value), a(a.value) {}

	int r;
	int g;
	int b;
	std::optional<int> a;
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

struct path
{
	explicit path(std::string str)
	: value(std::move(str)) {}

	explicit path(string s)
	: value(std::move(s.value)) {}

	std::string value;
};

inline bool operator==(const path& lhs, const path& rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(const path& lhs, const path& rhs) noexcept { return !(lhs == rhs); }

struct sound_id
{
	explicit sound_id(int value) : value(value) {}
	explicit sound_id(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(sound_id lhs, sound_id rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(sound_id lhs, sound_id rhs) noexcept { return !(lhs == rhs); }

struct volume
{
	explicit volume(int value) : value(value) {}
	explicit volume(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(volume lhs, volume rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(volume lhs, volume rhs) noexcept { return !(lhs == rhs); }

struct built_in_alert_sound
{
	explicit built_in_alert_sound(integer n)
	: id(n) {}

	explicit built_in_alert_sound(sound_id id)
	: id(id) {}

	explicit built_in_alert_sound(sound_id id, volume volume)
	: id(id), volume(volume) {}

	explicit built_in_alert_sound(sound_id id, boolean is_positional)
	: id(id), is_positional(is_positional) {}

	explicit built_in_alert_sound(sound_id id, volume volume, boolean is_positional)
	: id(id), volume(volume), is_positional(is_positional) {}

	sound_id id;
	std::optional<volume> volume;
	boolean is_positional = boolean{false};
};

inline bool operator==(built_in_alert_sound lhs, built_in_alert_sound rhs) noexcept
{
	return std::tie(lhs.id, lhs.volume, lhs.is_positional) == std::tie(rhs.id, rhs.volume, rhs.is_positional);
}
inline bool operator!=(built_in_alert_sound lhs, built_in_alert_sound rhs) noexcept { return !(lhs == rhs); }

struct custom_alert_sound
{
	explicit custom_alert_sound(string str)
	: path(std::move(str)) {}

	explicit custom_alert_sound(path path)
	: path(std::move(path)) {}

	path path;
};

inline bool operator==(const custom_alert_sound& lhs, const custom_alert_sound& rhs) noexcept { return lhs.path == rhs.path; }
inline bool operator!=(const custom_alert_sound& lhs, const custom_alert_sound& rhs) noexcept { return !(lhs == rhs); }

struct alert_sound
{
	explicit alert_sound(integer n)
	: sound(built_in_alert_sound(sound_id(n))) {}

	explicit alert_sound(sound_id id)
	: sound(built_in_alert_sound(id)) {}

	alert_sound(sound_id id, volume volume)
	: sound(built_in_alert_sound(id, volume)) {}

	alert_sound(sound_id id, boolean boolean)
	: sound(built_in_alert_sound(id, boolean)) {}

	alert_sound(sound_id id, volume volume, boolean boolean)
	: sound(built_in_alert_sound(id, volume, boolean)) {}

	explicit alert_sound(built_in_alert_sound sound)
	: sound(sound) {}

	explicit alert_sound(string s)
	: sound(custom_alert_sound(path(std::move(s)))) {}

	explicit alert_sound(path p)
	: sound(custom_alert_sound(std::move(p))) {}

	explicit alert_sound(custom_alert_sound sound)
	: sound(std::move(sound)) {}

	std::variant<built_in_alert_sound, custom_alert_sound> sound;
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

struct minimap_icon
{
	explicit minimap_icon(int size, suit color, shape shape)
	: size(integer{size}), color(color), shape(shape) {}

	explicit minimap_icon(integer size, suit color, shape shape)
	: size(size), color(color), shape(shape) {}

	integer size;
	suit color;
	shape shape;
};

inline bool operator==(minimap_icon lhs, minimap_icon rhs) noexcept
{
	return std::tie(lhs.size, lhs.color, lhs.shape) == std::tie(rhs.size, rhs.color, rhs.shape);
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

struct beam_effect
{
	explicit beam_effect(suit s)
	: color(s), is_temporary(boolean{false}) {}

	explicit beam_effect(suit s, boolean b)
	: color(s), is_temporary(b) {}

	suit color;
	boolean is_temporary;
};

inline bool operator==(beam_effect lhs, beam_effect rhs) noexcept
{
	return std::tie(lhs.color, lhs.is_temporary) == std::tie(rhs.color, rhs.is_temporary);
}
inline bool operator!=(beam_effect lhs, beam_effect rhs) noexcept { return !(lhs == rhs); }

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
