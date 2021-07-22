#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/position_tag.hpp>

#include <utility>
#include <tuple>
#include <iosfwd>
#include <optional>

namespace fs::lang
{

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

	explicit builtin_alert_sound_id(none n)
	: id(n) {}

	std::variant<integer, shaper_voice_line, none> id;
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
	explicit builtin_alert_sound(bool is_positional, builtin_alert_sound_id sound_id)
	: is_positional(is_positional), sound_id(sound_id) {}

	bool is_disabled() const { return std::holds_alternative<none>(sound_id.id); }

	bool is_positional; // this is not a lang type because it is implied by keyword, not by value
	builtin_alert_sound_id sound_id;
};

inline bool operator==(builtin_alert_sound lhs, builtin_alert_sound rhs) noexcept
{
	return std::tie(lhs.is_positional, lhs.sound_id) == std::tie(rhs.is_positional, rhs.sound_id);
}
inline bool operator!=(builtin_alert_sound lhs, builtin_alert_sound rhs) noexcept { return !(lhs == rhs); }

struct custom_alert_sound
{
	explicit custom_alert_sound(string path)
	: path(std::move(path)) {}

	bool is_disabled() const { return path.value.empty() || path.value == "None"; }

	string path;
};

inline bool operator==(const custom_alert_sound& lhs, const custom_alert_sound& rhs) noexcept { return lhs.path == rhs.path; }
inline bool operator!=(const custom_alert_sound& lhs, const custom_alert_sound& rhs) noexcept { return !(lhs == rhs); }

struct volume
{
	std::optional<integer> value;
};

inline bool operator==(const volume& lhs, const volume& rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(const volume& lhs, const volume& rhs) noexcept { return !(lhs == rhs); }

struct alert_sound
{
	explicit alert_sound(builtin_alert_sound sound, volume vol)
	: sound(sound), vol(vol) {}

	explicit alert_sound(custom_alert_sound sound, volume vol)
	: sound(std::move(sound)), vol(vol) {}

	bool is_disabled() const
	{
		return std::visit([](auto sound) { return sound.is_disabled(); }, sound);
	}

	std::variant<builtin_alert_sound, custom_alert_sound> sound;
	volume vol;
};

inline bool operator==(const alert_sound& lhs, const alert_sound& rhs) noexcept
{
	return std::tie(lhs.sound, lhs.vol) == std::tie(rhs.sound, rhs.vol);
}
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

struct switch_drop_sound_action
{
	switch_drop_sound action;
	position_tag origin;
};

inline bool operator==(switch_drop_sound_action lhs, switch_drop_sound_action rhs)
{
	return lhs.action == rhs.action;
}

inline bool operator!=(switch_drop_sound_action lhs, switch_drop_sound_action rhs)
{
	return !(lhs == rhs);
}

struct enabled_minimap_icon
{
	enabled_minimap_icon(integer size, suit color, shape shape_)
	: size(size), color(color), shape_(shape_) {}

	integer size;
	suit color;
	shape shape_;
};

inline bool operator==(enabled_minimap_icon lhs, enabled_minimap_icon rhs) noexcept
{
	return std::tie(lhs.size, lhs.color, lhs.shape_) == std::tie(rhs.size, rhs.color, rhs.shape_);
}
inline bool operator!=(enabled_minimap_icon lhs, enabled_minimap_icon rhs) noexcept { return !(lhs == rhs); }

struct disabled_minimap_icon
{
	position_tag sentinel_origin;
};

inline bool operator==(disabled_minimap_icon /* lhs */, disabled_minimap_icon /* rhs */) noexcept
{
	return true;
}
inline bool operator!=(disabled_minimap_icon lhs, disabled_minimap_icon rhs) noexcept { return !(lhs == rhs); }

struct minimap_icon
{
	static constexpr int sentinel_cancel_value = -1;

	std::variant<enabled_minimap_icon, disabled_minimap_icon> icon;
	position_tag expr_origin;
};

inline bool operator==(minimap_icon lhs, minimap_icon rhs) noexcept
{
	return lhs.icon == rhs.icon;
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

struct enabled_play_effect
{
	enabled_play_effect(suit s, bool is_temporary)
	: color(s), is_temporary(is_temporary) {}

	suit color;
	bool is_temporary;
};

inline bool operator==(enabled_play_effect lhs, enabled_play_effect rhs) noexcept
{
	return std::tie(lhs.color, lhs.is_temporary) == std::tie(rhs.color, rhs.is_temporary);
}
inline bool operator!=(enabled_play_effect lhs, enabled_play_effect rhs) noexcept { return !(lhs == rhs); }

struct disabled_play_effect
{
	none none_;
};

inline bool operator==(disabled_play_effect lhs, disabled_play_effect rhs) noexcept
{
	return lhs.none_ == rhs.none_;
}
inline bool operator!=(disabled_play_effect lhs, disabled_play_effect rhs) noexcept { return !(lhs == rhs); }

struct play_effect
{
	std::variant<enabled_play_effect, disabled_play_effect> effect;
	position_tag origin;
};

inline bool operator==(play_effect lhs, play_effect rhs) noexcept
{
	return lhs.effect == rhs.effect;
}
inline bool operator!=(play_effect lhs, play_effect rhs) noexcept { return !(lhs == rhs); }

struct play_effect_action
{
	play_effect effect;
	position_tag origin;
};

inline bool operator==(play_effect_action lhs, play_effect_action rhs)
{
	return lhs.effect == rhs.effect;
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
	void override_with(const action_set& other);

	void generate(std::ostream& output_stream) const;

	std::optional<color_action> set_border_color;
	std::optional<color_action> set_text_color;
	std::optional<color_action> set_background_color;
	std::optional<font_size_action> set_font_size;
	std::optional<alert_sound_action> play_alert_sound;
	std::optional<switch_drop_sound_action> switch_drop_sound;
	std::optional<minimap_icon_action> minimap_icon;
	std::optional<play_effect_action> play_effect;
};

bool operator==(const action_set& lhs, const action_set& rhs);
bool operator!=(const action_set& lhs, const action_set& rhs);

}
