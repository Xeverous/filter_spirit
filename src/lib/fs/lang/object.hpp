#pragma once

#include <fs/utility/type_traits.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/action_set.hpp>

#include <vector>
#include <variant>

namespace fs::lang
{

class object;

using array_object = std::vector<object>;

using object_variant = std::variant<
	// primitive types
	none,
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
	array_object,
	// compound action
	action_set
>;

BETTER_ENUM(object_type, int,
	// primitive types
	none,
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
	array,
	// compound action
	action_set)

struct object
{
	bool is_array() const noexcept
	{
		return std::holds_alternative<array_object>(value);
	}

	bool is_compound_action() const noexcept
	{
		return std::holds_alternative<action_set>(value);
	}

	bool is_primitive() const noexcept
	{
		return !is_array() && !is_compound_action();
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
object_type type_to_enum_impl<none>() { return object_type::none; }
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
template <> constexpr
object_type type_to_enum_impl<action_set>() { return object_type::action_set; }

template <typename T> [[nodiscard]] constexpr
object_type type_to_enum()
{
	static_assert(
		traits::is_variant_alternative_v<T, object_variant>,
		"T must be one of object type alternatives");

	return type_to_enum_impl<T>();
}

}
