#pragma once

#include <tuple>
#include <optional>
#include <utility>
#include <string>
#include <variant>

namespace fs::lang
{

struct none {};

inline bool operator==(none, none) noexcept { return true; }
inline bool operator!=(none, none) noexcept { return false; }

struct boolean
{
	bool value;
};

inline bool operator==(boolean lhs, boolean rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(boolean lhs, boolean rhs) noexcept { return !(lhs == rhs); }

struct integer
{
	int value;
};

inline bool operator==(integer lhs, integer rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(integer lhs, integer rhs) noexcept { return !(lhs == rhs); }

struct floating_point
{
	explicit floating_point(double value) : value(value) {}
	explicit floating_point(integer n) : value(n.value) {}

	double value;
};

inline bool operator==(floating_point lhs, floating_point rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(floating_point lhs, floating_point rhs) noexcept { return !(lhs == rhs); }

struct level
{
	explicit level(int value) : value(value) {}
	explicit level(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(level lhs, level rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(level lhs, level rhs) noexcept { return !(lhs == rhs); }

struct font_size
{
	explicit font_size(int value) : value(value) {}
	explicit font_size(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(font_size lhs, font_size rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(font_size lhs, font_size rhs) noexcept { return !(lhs == rhs); }

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

struct socket_group
{
	bool is_valid() const noexcept
	{
		int sum = r + g + b + w;
		return 0 < sum && sum <= 6;
	}

	int r = 0;
	int g = 0;
	int b = 0;
	int w = 0;
};

inline bool operator==(socket_group lhs, socket_group rhs) noexcept
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.w) == std::tie(rhs.r, rhs.g, rhs.b, rhs.w);
}
inline bool operator!=(socket_group lhs, socket_group rhs) noexcept { return !(lhs == rhs); }

enum class influence { shaper, elder, crusader, redeemer, hunter, warlord };
enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
// suit is named "suit" to avoid name conflict with "color"
enum class suit { red, green, blue, white, brown, yellow };

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

struct string
{
	std::string value;
};

inline bool operator==(const string& lhs, const string& rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(const string& lhs, const string& rhs) noexcept { return !(lhs == rhs); }

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

} // namespace fs::lang
