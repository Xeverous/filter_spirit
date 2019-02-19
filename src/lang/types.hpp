#pragma once
#include "lang/type_names.hpp"
#include "parser/config.hpp"
#include "utility/type_traits.hpp"
#include <cassert>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

namespace fs::lang
{

// ---- TODO sort/implement/refactor ---

enum class comparison_type
{
	less,
	less_equal,
	equal,
	greater,
	greater_equal
};

struct built_in_sound
{
	sound_id id;
	bool positional;
};

struct custom_alert_sound
{
	std::string path;
};

struct alert_sound
{
	std::variant<built_in_sound, custom_alert_sound> sound;
	volume vol;
};

// ---- new syntax ----

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
	shaped_map
};

enum class action_type
{
	set_border_color,
	set_text_color,
	set_background_color
	// TODO add missing action types
};

struct boolean
{
	bool value;
};

struct integer
{
	int value;
};

struct level
{
	explicit level(integer n) : value(n.value) {}

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
	std::string value;
};

using single_object = std::variant<
	boolean,
	integer,
	level,
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
	path
>;

enum class single_object_type
{
	boolean,
	integer,
	level,
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

	generic // for generic constructs such as empty arrays
};

struct object_type
{
	object_type(single_object_type type, bool is_array = false)
	: type(type), is_array(is_array)
	{
	}

	single_object_type type;
	bool is_array;
};

[[nodiscard]]
std::string_view to_string(single_object_type type);
[[nodiscard]]
std::string to_string(object_type type);

using position_tag = boost::spirit::x3::position_tagged;

using array_object = std::vector<struct object>;

struct object
{
	bool is_array() const
	{
		return std::holds_alternative<array_object>(value);
	}

	object promote_to_array() const
	{
		assert(!is_array());
		return object{
			array_object(1, *this),
			value_origin,
			name_origin};
	}

	std::variant<single_object, array_object> value;
	// for printing errors about mismatched types
	position_tag value_origin;
	// for printing error name already exists
	// if object originated from a literal this should be empty
	std::optional<position_tag> name_origin;
};

[[nodiscard]]
single_object_type type_of_single_object(const single_object& object);
[[nodiscard]]
object_type type_of_object(const object& object);
[[nodiscard]]
object_type type_of_object(const single_object& object);

template <typename T> [[nodiscard]] constexpr
single_object_type type_to_enum_impl()
{
	static_assert(sizeof(T) == 0, "missing implementation for this type");
	return single_object_type::generic;
}

template <> constexpr
single_object_type type_to_enum_impl<boolean>() { return single_object_type::boolean; }
template <> constexpr
single_object_type type_to_enum_impl<integer>() { return single_object_type::integer; }
template <> constexpr
single_object_type type_to_enum_impl<level>() { return single_object_type::level; }
template <> constexpr
single_object_type type_to_enum_impl<sound_id>() { return single_object_type::sound_id; }
template <> constexpr
single_object_type type_to_enum_impl<volume>() { return single_object_type::volume; }
template <> constexpr
single_object_type type_to_enum_impl<socket_group>() { return single_object_type::socket_group; }
template <> constexpr
single_object_type type_to_enum_impl<rarity>() { return single_object_type::rarity; }
template <> constexpr
single_object_type type_to_enum_impl<shape>() { return single_object_type::shape; }
template <> constexpr
single_object_type type_to_enum_impl<suit>() { return single_object_type::suit; }
template <> constexpr
single_object_type type_to_enum_impl<color>() { return single_object_type::color; }
template <> constexpr
single_object_type type_to_enum_impl<minimap_icon>() { return single_object_type::minimap_icon; }
template <> constexpr
single_object_type type_to_enum_impl<beam_effect>() { return single_object_type::beam_effect; }
template <> constexpr
single_object_type type_to_enum_impl<string>() { return single_object_type::string; }
template <> constexpr
single_object_type type_to_enum_impl<path>() { return single_object_type::path; }

template <typename T> [[nodiscard]] constexpr
single_object_type type_to_enum()
{
	static_assert(
		traits::is_variant_alternative_v<T, single_object>,
		"T must be one of object type alternatives");

	return type_to_enum_impl<T>();
}

}
