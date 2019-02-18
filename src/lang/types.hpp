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

enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
enum class suit { red, green, blue, white, brown, yellow };

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
	int value;
	parser::range_type origin;
};

struct sound_id
{
	int value;
};

struct volume
{
	int value;
};

struct color
{
	int r;
	int g;
	int b;
	std::optional<int> a;
	parser::range_type origin;
};

struct group
{
	int r = 0;
	int g = 0;
	int b = 0;
	int w = 0;
};

struct string
{
	std::string value;
};

using single_object = std::variant<
	boolean,
	integer,
	level,
	sound_id,
	volume,
	color,
	rarity,
	shape,
	suit,
	group,
	string
>;

// ----

enum class comparison_type
{
	less,
	less_equal,
	equal,
	greater,
	greater_equal
};

// ---- conditions ----

// ---- actions ----

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

struct minimap_icon
{
	int size;
	suit color;
	shape shape;
};

struct beam_effect
{
	suit color;
	bool is_temporary;
};

// ---- utility functions ----
[[nodiscard]]
single_object_type type_of_single_object(const single_object& obj);
[[nodiscard]]
object_type type_of_object(const struct object& obj);

template <typename T> [[nodiscard]] constexpr
single_object_type type_to_enum_impl()
{
	static_assert(sizeof(T) == 0, "missing implementation for this type");
	return single_object_type::generic;
}

template <> constexpr
single_object_type type_to_enum_impl<boolean>() { return single_object_type::boolean; }
template <> constexpr
single_object_type type_to_enum_impl<integer>() { return single_object_type::number; }
template <> constexpr
single_object_type type_to_enum_impl<level>() { return single_object_type::level; }
template <> constexpr
single_object_type type_to_enum_impl<sound_id>() { return single_object_type::sound_id; }
template <> constexpr
single_object_type type_to_enum_impl<volume>() { return single_object_type::volume; }
template <> constexpr
single_object_type type_to_enum_impl<color>() { return single_object_type::color; }
template <> constexpr
single_object_type type_to_enum_impl<rarity>() { return single_object_type::rarity; }
template <> constexpr
single_object_type type_to_enum_impl<shape>() { return single_object_type::shape; }
template <> constexpr
single_object_type type_to_enum_impl<suit>() { return single_object_type::suit; }
template <> constexpr
single_object_type type_to_enum_impl<group>() { return single_object_type::group; }
template <> constexpr
single_object_type type_to_enum_impl<string>() { return single_object_type::string; }

template <typename T> [[nodiscard]] constexpr
single_object_type type_to_enum()
{
	static_assert(
		traits::is_variant_alternative_v<T, single_object>,
		"T must be one of object type alternatives");

	return type_to_enum_impl<T>();
}

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

using position_tag = boost::spirit::x3::position_tagged;

using array_object = std::vector<struct object>;

struct object
{
	bool is_array() const
	{
		return std::holds_alternative<array_object>(value);
	}

	std::variant<single_object, array_object> value;
	// for printing errors about mismatched types
	position_tag value_origin;
	// for printing error name already exists
	// if object originated from a literal this should be empty
	std::optional<position_tag> name_origin;
};

}
