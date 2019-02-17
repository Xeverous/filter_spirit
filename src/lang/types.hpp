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
#include <unordered_map>

namespace fs::lang
{

enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
enum class suit { red, green, blue, white, brown, yellow };

struct boolean
{
	bool value;
};

struct number
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
	number,
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

using array_object = std::vector<single_object>;

struct object
{
	std::variant<single_object, array_object> value;
	// for printing errors about mismatched types
	// if object_ originated from a literal this should point at the literal
	parser::range_type type_origin;
	// if object_ was referened from other object_ this should point at the use of
	// that object_ - in other words, point at expression which object_ was assigned from
	parser::range_type value_origin;
	// for printing error name already exists
	// if object originated from a literal this should be empty
	std::optional<fs::parser::range_type> name_origin;
};

using constants_map = std::unordered_map<std::string, object>;

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
object_type type_of_object(const object& obj);

template <typename T> [[nodiscard]] constexpr
single_object_type type_to_enum_impl()
{
	static_assert(sizeof(T) == 0, "missing implementation for this type");
	return single_object_type::generic;
}

template <> constexpr
single_object_type type_to_enum_impl<boolean>() { return single_object_type::boolean; }
template <> constexpr
single_object_type type_to_enum_impl<number>() { return single_object_type::number; }
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

enum class action_type
{
	set_border_color,
	set_text_color,
	set_background_color
	// TODO add missing action types
};

}
