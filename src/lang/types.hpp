#pragma once
#include "parser/config.hpp"
#include "utility/type_traits.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>

namespace fs::lang
{

// ---- constants ----

enum class single_object_type
{
	boolean,
	number,
	level,
	sound_id,
	volume,
	rarity,
	shape,
	suit,
	color,
	group,
	string,

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

template <typename T>
struct range_bound
{
	T value;
	bool inclusive;
};

template <typename T>
struct range
{
	std::optional<range_bound<T>> lower_bound;
	std::optional<range_bound<T>> upper_bound;
};

template <typename T>
struct exact_value
{
	T value;
};

template <typename T>
struct range_condition
{
	std::variant<range<T>, exact_value<T>> spec;
	parser::range_type origin;
};

using numeric_range_condition = range_condition<int>;
using rarity_range_condition = range_condition<rarity>;

struct condition_set
{
	std::optional<numeric_range_condition> item_level;
	std::optional<numeric_range_condition> drop_level;
	std::optional<numeric_range_condition> quality;
	std::optional<rarity_range_condition> rarity;
	// ??? class_;
	// ??? base_type;
	std::optional<numeric_range_condition> sockets;
	std::optional<numeric_range_condition> links;
	std::optional<group> socket_group;
	std::optional<numeric_range_condition> height;
	std::optional<numeric_range_condition> width;
	// ??? has_explicit_mod;
	std::optional<numeric_range_condition> stack_size;
	std::optional<numeric_range_condition> gem_level;
	std::optional<numeric_range_condition> map_tier;
	std::optional<bool> is_identified;
	std::optional<bool> id_corrupted;
	std::optional<bool> is_shaper_item;
	std::optional<bool> is_elder_item;
	std::optional<bool> is_shaped_map;
};

// ---- actions ----

struct built_in_sound
{
	sound_id id;
	bool positional;
};

struct custom_alert_sound
{
	std::string_view path;
};

struct alert_sound
{
	std::variant<built_in_sound, custom_alert_sound>;
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

struct action_set
{
	std::optional<bool> show;
	std::optional<color> border_color;
	std::optional<color> text_color;
	std::optional<color> background_color;
	std::optional<number> font_size;
	std::optional<alert_sound> sound;
	std::optional<std::monostate> disabled_drop_sound;
	std::optional<minimap_icon> icon;
	std::optional<beam_effect> effect;

	[[nodiscard]]
	action_set override_by(const action_set& other) const;
};

// ---- filter ----

struct filter_block
{
	condition_set conditions;
	action_set actions;
};

// ---- utility functions ----
[[nodiscard]]
std::string_view to_string(single_object_type type);
[[nodiscard]]
std::string to_string(object_type type);

[[nodiscard]]
single_object_type type_of_single_object(const single_object& obj);
[[nodiscard]]
object_type type_of_object(const object& obj);

[[nodiscard]]
template <typename T> constexpr
single_object_type type_to_enum_impl()
{
	static_assert(sizeof(T) == 0, "missing implementation for this type");
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

[[nodiscard]]
template <typename T> constexpr
single_object_type type_to_enum()
{
	static_assert(
		traits::is_variant_alternative_v<T, single_object>,
		"T must be one of object type alternatives");

	return type_to_enum_impl<T>();
}

}
