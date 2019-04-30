#pragma once

#include "fs/utility/type_traits.hpp"
#include "fs/utility/better_enum.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
#include <map>

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
	has_explicit_mod
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

enum class action_type
{
	set_border_color,
	set_text_color,
	set_background_color,
	set_font_size,
	set_alert_sound,
	set_alert_sound_positional,
	disable_drop_sound,
	set_minimap_icon,
	set_beam
};

struct boolean
{
	bool value;
};

struct integer
{
	int value;
};

// should be above integer for consistency but it has a dependency in ctor
struct floating_point
{
	explicit floating_point(double value) : value(value) {}
	explicit floating_point(integer n) : value(n.value) {}

	double value;
};

struct level
{
	explicit level(integer n) : value(n.value) {}

	int value;
};

struct quality
{
	explicit quality(integer n) : value(n.value) {}

	int value;
};

struct font_size
{
	explicit font_size(integer n) : value(n.value) {}

	int value;
};

struct sound_id
{
	explicit sound_id(integer n) : value(n.value) {}

	int value;
};

struct volume
{
	explicit volume(integer n) : value(n.value) {}

	int value;
};

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

enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
enum class suit { red, green, blue, white, brown, yellow };

struct color
{
	color(int r, int g, int b)
	: r(r), g(g), b(b) {}

	color(int r, int g, int b, int a)
	: r(r), g(g), b(b), a(a) {}

	int r;
	int g;
	int b;
	std::optional<int> a;
};

struct minimap_icon
{
	int size;
	suit color;
	shape shape;
};

struct beam_effect
{
	explicit beam_effect(suit s)
	: color(s), is_temporary(false) {}

	suit color;
	bool is_temporary;
};

struct string
{
	std::string value;
};

struct path
{
	explicit path(string s)
	: value(std::move(s.value)) {}

	std::string value;
};

struct alert_sound
{
	explicit alert_sound(sound_id id)
	: sound(id) {}

	explicit alert_sound(integer n)
	: sound(sound_id(n)) {}

	explicit alert_sound(path p)
	: sound(std::move(p)) {}

	explicit alert_sound(string s)
	: sound(path(std::move(s))) {}

	std::variant<sound_id, path> sound;
	std::optional<volume> vol;
};

class object;

using array_object = std::vector<object>;
using dict_object = std::map<std::string, object>;

using object_variant = std::variant<
	// primitive types
	boolean,
	floating_point,
	integer,
	level,
	quality,
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
	alert_sound,
	// array
	array_object,
	// dictionary
	dict_object
>;

BETTER_ENUM(object_type, int,
	// primitive types
	boolean,
	floating_point,
	integer,
	level,
	quality,
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
	alert_sound,
	// array
	array,
	// dictionary
	dictionary)

using position_tag = boost::spirit::x3::position_tagged;

struct object
{
	bool is_array() const noexcept
	{
		return std::holds_alternative<array_object>(value);
	}

	bool is_dict() const noexcept
	{
		return std::holds_alternative<dict_object>(value);
	}

	bool is_structured() const noexcept
	{
		return is_array() || is_dict();
	}

	bool is_primitive() const noexcept
	{
		return !is_structured();
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

[[nodiscard]]
std::string_view to_string_view(object_type type);

[[nodiscard]]
object_type type_of_object(const object& object);

template <typename T> [[nodiscard]] constexpr
object_type type_to_enum_impl()
{
	static_assert(sizeof(T) == 0, "missing implementation for this type");
	return object_type::dictionary;
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
object_type type_to_enum_impl<quality>() { return object_type::quality; }
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
object_type type_to_enum_impl<alert_sound>() { return object_type::alert_sound; }
template <> constexpr
object_type type_to_enum_impl<array_object>() { return object_type::array; }
template <> constexpr
object_type type_to_enum_impl<dict_object>() { return object_type::dictionary; }

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
