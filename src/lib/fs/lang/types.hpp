#pragma once

#include <fs/utility/type_traits.hpp>
#include <fs/utility/better_enum.hpp>

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace fs::lang
{

enum class comparison_type
{
	less,
	less_equal,
	equal,
	greater,
	greater_equal
};

enum class comparison_condition_property
{
	item_level,
	drop_level,
	quality,
	rarity,
	sockets,
	links,
	height,
	width,
	stack_size,
	gem_level,
	map_tier
};

enum class string_condition_property
{
	class_,
	base_type,
	has_explicit_mod,
	has_enchantment
};

enum class boolean_condition_property
{
	identified,
	corrupted,
	elder_item,
	shaper_item,
	fractured_item,
	synthesised_item,
	any_enchantment,
	shaped_map
};

enum class nullary_action_type
{
	enable_drop_sound,
	disable_drop_sound
};

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

struct boolean
{
	bool value;
};

inline bool operator==(boolean lhs, boolean rhs) { return lhs.value == rhs.value; }
inline bool operator!=(boolean lhs, boolean rhs) { return !(lhs == rhs); }

struct integer
{
	int value;
};

inline bool operator==(integer lhs, integer rhs) { return lhs.value == rhs.value; }
inline bool operator!=(integer lhs, integer rhs) { return !(lhs == rhs); }

// should be above integer for consistency but it has a dependency in ctor
struct floating_point
{
	explicit floating_point(double value) : value(value) {}
	explicit floating_point(integer n) : value(n.value) {}

	double value;
};

inline bool operator==(floating_point lhs, floating_point rhs) { return lhs.value == rhs.value; }
inline bool operator!=(floating_point lhs, floating_point rhs) { return !(lhs == rhs); }

struct level
{
	explicit level(int value) : value(value) {}
	explicit level(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(level lhs, level rhs) { return lhs.value == rhs.value; }
inline bool operator!=(level lhs, level rhs) { return !(lhs == rhs); }

struct font_size
{
	explicit font_size(int value) : value(value) {}
	explicit font_size(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(font_size lhs, font_size rhs) { return lhs.value == rhs.value; }
inline bool operator!=(font_size lhs, font_size rhs) { return !(lhs == rhs); }

struct sound_id
{
	explicit sound_id(int value) : value(value) {}
	explicit sound_id(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(sound_id lhs, sound_id rhs) { return lhs.value == rhs.value; }
inline bool operator!=(sound_id lhs, sound_id rhs) { return !(lhs == rhs); }

struct volume
{
	explicit volume(int value) : value(value) {}
	explicit volume(integer n) : value(n.value) {}

	int value;
};

inline bool operator==(volume lhs, volume rhs) { return lhs.value == rhs.value; }
inline bool operator!=(volume lhs, volume rhs) { return !(lhs == rhs); }

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

inline bool operator==(socket_group lhs, socket_group rhs)
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.w) == std::tie(rhs.r, rhs.g, rhs.b, rhs.w);
}
inline bool operator!=(socket_group lhs, socket_group rhs) { return !(lhs == rhs); }

enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
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

inline bool operator==(color lhs, color rhs)
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.a) == std::tie(rhs.r, rhs.g, rhs.b, rhs.a);
}
inline bool operator!=(color lhs, color rhs) { return !(lhs == rhs); }

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

inline bool operator==(minimap_icon lhs, minimap_icon rhs)
{
	return std::tie(lhs.size, lhs.color, lhs.shape) == std::tie(rhs.size, rhs.color, rhs.shape);
}
inline bool operator!=(minimap_icon lhs, minimap_icon rhs) { return !(lhs == rhs); }

struct beam_effect
{
	explicit beam_effect(suit s)
	: color(s), is_temporary(false) {}

	explicit beam_effect(suit s, boolean b)
	: color(s), is_temporary(b.value) {}

	suit color;
	bool is_temporary;
};

inline bool operator==(beam_effect lhs, beam_effect rhs)
{
	return std::tie(lhs.color, lhs.is_temporary) == std::tie(rhs.color, rhs.is_temporary);
}
inline bool operator!=(beam_effect lhs, beam_effect rhs) { return !(lhs == rhs); }

struct string
{
	std::string value;
};

inline bool operator==(const string& lhs, const string& rhs) { return lhs.value == rhs.value; }
inline bool operator!=(const string& lhs, const string& rhs) { return !(lhs == rhs); }

struct path
{
	explicit path(std::string str)
	: value(std::move(str)) {}

	explicit path(string s)
	: value(std::move(s.value)) {}

	std::string value;
};

inline bool operator==(const path& lhs, const path& rhs) { return lhs.value == rhs.value; }
inline bool operator!=(const path& lhs, const path& rhs) { return !(lhs == rhs); }

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

inline bool operator==(built_in_alert_sound lhs, built_in_alert_sound rhs)
{
	return std::tie(lhs.id, lhs.volume, lhs.is_positional) == std::tie(rhs.id, rhs.volume, rhs.is_positional);
}
inline bool operator!=(built_in_alert_sound lhs, built_in_alert_sound rhs) { return !(lhs == rhs); }

struct custom_alert_sound
{
	explicit custom_alert_sound(string str)
	: path(std::move(str)) {}

	explicit custom_alert_sound(path path)
	: path(std::move(path)) {}

	path path;
};

inline bool operator==(const custom_alert_sound& lhs, const custom_alert_sound& rhs) { return lhs.path == rhs.path; }
inline bool operator!=(const custom_alert_sound& lhs, const custom_alert_sound& rhs) { return !(lhs == rhs); }

struct alert_sound
{
	explicit alert_sound(integer n)
	: sound(built_in_alert_sound(sound_id(n))) {}

	explicit alert_sound(sound_id id)
	: sound(built_in_alert_sound(id)) {}

	explicit alert_sound(sound_id id, volume volume)
	: sound(built_in_alert_sound(id, volume)) {}

	explicit alert_sound(sound_id id, boolean boolean)
	: sound(built_in_alert_sound(id, boolean)) {}

	explicit alert_sound(sound_id id, volume volume, boolean boolean)
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

inline bool operator==(const alert_sound& lhs, const alert_sound& rhs) { return lhs.sound == rhs.sound; }
inline bool operator!=(const alert_sound& lhs, const alert_sound& rhs) { return !(lhs == rhs); }

class object;

using array_object = std::vector<object>;

using object_variant = std::variant<
	// primitive types
	boolean,
	floating_point,
	integer,
	level,
	font_size,
	sound_id,
	volume,
	socket_group,
	rarity,
	shape,
	suit,
	color,
	minimap_icon,
	beam_effect,
	string,
	path,
	built_in_alert_sound,
	custom_alert_sound,
	alert_sound,
	// array
	array_object
>;

BETTER_ENUM(object_type, int,
	// primitive types
	boolean,
	floating_point,
	integer,
	level,
	font_size,
	sound_id,
	volume,
	socket_group,
	rarity,
	shape,
	suit,
	color,
	minimap_icon,
	beam_effect,
	string,
	path,
	built_in_alert_sound,
	custom_alert_sound,
	alert_sound,
	// array
	array)

using position_tag = boost::spirit::x3::position_tagged;

struct object
{
	bool is_array() const noexcept
	{
		return std::holds_alternative<array_object>(value);
	}

	bool is_primitive() const noexcept
	{
		return !is_array();
	}

	[[nodiscard]]
	array_object promote_to_array() const
	{
		assert(!is_array());
		return array_object(1, *this);
	}

	object_variant value;
	position_tag value_origin;
};

inline bool operator==(const object& lhs, const object& rhs)
{
	// we intentionally do not compare origins
	// this operator is used by tests and potentially in the language
	return lhs.value == rhs.value;
}
inline bool operator!=(const object& lhs, const object& rhs) { return !(lhs == rhs); }


[[nodiscard]]
std::string_view to_string_view(object_type type);

[[nodiscard]]
object_type type_of_object(const object_variant& object);

template <typename T> [[nodiscard]] constexpr
object_type type_to_enum_impl()
{
	static_assert(sizeof(T) == 0, "missing implementation for this type");
	return object_type::array; // return statement only to silence compiler/editors
}

template <> constexpr
object_type type_to_enum_impl<boolean>() { return object_type::boolean; }
template <> constexpr
object_type type_to_enum_impl<floating_point>() { return object_type::floating_point; }
template <> constexpr
object_type type_to_enum_impl<integer>() { return object_type::integer; }
template <> constexpr
object_type type_to_enum_impl<level>() { return object_type::level; }
template <> constexpr
object_type type_to_enum_impl<font_size>() { return object_type::font_size; }
template <> constexpr
object_type type_to_enum_impl<sound_id>() { return object_type::sound_id; }
template <> constexpr
object_type type_to_enum_impl<volume>() { return object_type::volume; }
template <> constexpr
object_type type_to_enum_impl<socket_group>() { return object_type::socket_group; }
template <> constexpr
object_type type_to_enum_impl<rarity>() { return object_type::rarity; }
template <> constexpr
object_type type_to_enum_impl<shape>() { return object_type::shape; }
template <> constexpr
object_type type_to_enum_impl<suit>() { return object_type::suit; }
template <> constexpr
object_type type_to_enum_impl<color>() { return object_type::color; }
template <> constexpr
object_type type_to_enum_impl<minimap_icon>() { return object_type::minimap_icon; }
template <> constexpr
object_type type_to_enum_impl<beam_effect>() { return object_type::beam_effect; }
template <> constexpr
object_type type_to_enum_impl<string>() { return object_type::string; }
template <> constexpr
object_type type_to_enum_impl<path>() { return object_type::path; }
template <> constexpr
object_type type_to_enum_impl<built_in_alert_sound>() { return object_type::built_in_alert_sound; }
template <> constexpr
object_type type_to_enum_impl<custom_alert_sound>() { return object_type::custom_alert_sound; }
template <> constexpr
object_type type_to_enum_impl<alert_sound>() { return object_type::alert_sound; }
template <> constexpr
object_type type_to_enum_impl<array_object>() { return object_type::array; }

template <typename T> [[nodiscard]] constexpr
object_type type_to_enum()
{
	static_assert(
		traits::is_variant_alternative_v<T, object_variant>,
		"T must be one of object type alternatives");

	return type_to_enum_impl<T>();
}

struct price_range
{
	double min;
	double max;
};

}
