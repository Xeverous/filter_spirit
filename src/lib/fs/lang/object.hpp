#pragma once

#include <fs/utility/type_traits.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/action_set.hpp>
#include <fs/lang/queries.hpp>

#include <boost/container/small_vector.hpp>

#include <variant>

namespace fs::lang
{

using primitive_object_variant = std::variant<
	none,
	underscore,
	boolean,
	floating_point,
	integer,
	socket_group,
	influence,
	rarity,
	shape,
	suit,
	string
>;

struct primitive_object
{
	primitive_object_variant value;
	position_tag origin;
};

inline bool operator==(const primitive_object& lhs, const primitive_object& rhs)
{
	return lhs.value == rhs.value;
}

inline bool operator!=(const primitive_object& lhs, const primitive_object& rhs)
{
	return !(lhs == rhs);
}

struct sequence_object
{
	// Use small_vector because most lang sequences will have very lmited number of elements -
	// the longest sequences are color (R, G, B, + optonal A) and arrays of strings for string-based
	// conditions like BaseType.
	// This container optimizes storage so that only long arrays of strings will allocate.
	using container_type = boost::container::small_vector<primitive_object, 4>;

	container_type values;
	position_tag  origin;
};

inline bool operator==(const sequence_object& lhs, const sequence_object& rhs)
{
	return lhs.values == rhs.values;
}

inline bool operator!=(const sequence_object& lhs, const sequence_object& rhs)
{
	return !(lhs == rhs);
}

using object_variant = std::variant<sequence_object, action_set, query>;

BETTER_ENUM(primitive_object_type, int,
	none,
	underscore,
	boolean,
	floating_point,
	integer,
	socket_group,
	influence,
	rarity,
	shape,
	suit,
	string)

BETTER_ENUM(object_type, int, sequence, compound_action, query)

[[nodiscard]] inline
std::string_view to_string_view(primitive_object_type type) noexcept
{
	return type._to_string();
}

[[nodiscard]] inline
std::string_view to_string_view(object_type type) noexcept
{
	return type._to_string();
}

[[nodiscard]]
object_type type_of_object(const object_variant& obj) noexcept;
[[nodiscard]]
primitive_object_type type_of_primitive(const primitive_object_variant& obj) noexcept;

struct object
{
	bool is_sequence() const noexcept
	{
		return std::holds_alternative<sequence_object>(value);
	}

	bool is_compound_action() const noexcept
	{
		return std::holds_alternative<action_set>(value);
	}

	bool is_query() const noexcept
	{
		return std::holds_alternative<query>(value);
	}

	object_type type() const noexcept
	{
		return type_of_object(value);
	}

	object_variant value;
	position_tag origin;
};

bool operator==(const object& lhs, const object& rhs) noexcept;

inline
bool operator!=(const object& lhs, const object& rhs) noexcept { return !(lhs == rhs); }

namespace detail {
	template <typename T> [[nodiscard]] constexpr
	primitive_object_type type_to_enum_impl() noexcept
	{
		static_assert(sizeof(T) == 0, "missing implementation for this type");
		return primitive_object_type::none; // return statement only to silence compiler/editors
	}

	template <> constexpr
	primitive_object_type type_to_enum_impl<none>() noexcept { return primitive_object_type::none; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<underscore>() noexcept { return primitive_object_type::underscore; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<boolean>() noexcept { return primitive_object_type::boolean; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<floating_point>() noexcept { return primitive_object_type::floating_point; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<integer>() noexcept { return primitive_object_type::integer; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<socket_group>() noexcept { return primitive_object_type::socket_group; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<influence>() noexcept { return primitive_object_type::influence; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<rarity>() noexcept { return primitive_object_type::rarity; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<shape>() noexcept { return primitive_object_type::shape; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<suit>() noexcept { return primitive_object_type::suit; }
	template <> constexpr
	primitive_object_type type_to_enum_impl<string>() noexcept { return primitive_object_type::string; }
}

template <typename T> [[nodiscard]] constexpr
primitive_object_type type_to_enum() noexcept
{
	static_assert(
		traits::is_variant_alternative_v<T, primitive_object_variant>,
		"T must be one of object type alternatives");

	return detail::type_to_enum_impl<T>();
}

}
