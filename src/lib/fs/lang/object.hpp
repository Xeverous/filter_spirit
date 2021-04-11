#pragma once

#include <fs/utility/type_traits.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/queries.hpp>

#include <boost/container/small_vector.hpp>

#include <variant>

namespace fs::lang
{

using object_variant = std::variant<
	none,
	temp,
	boolean,
	fractional,
	integer,
	socket_spec,
	influence,
	rarity,
	shape,
	suit,
	shaper_voice_line,
	gem_quality_type,
	string
>;

BETTER_ENUM(object_type, int,
	none,
	temp,
	boolean,
	fractional,
	integer,
	socket_spec,
	influence,
	rarity,
	shape,
	suit,
	shaper_voice_line,
	gem_quality_type,
	string)


[[nodiscard]]
object_type object_type_of(const object_variant& obj) noexcept;

struct single_object
{
	object_type type() const noexcept
	{
		return object_type_of(value);
	}

	object_variant value;
	position_tag origin;
};

inline bool operator==(const single_object& lhs, const single_object& rhs)
{
	return lhs.value == rhs.value;
}

inline bool operator!=(const single_object& lhs, const single_object& rhs)
{
	return !(lhs == rhs);
}

[[nodiscard]] inline
std::string_view to_string_view(object_type type) noexcept
{
	return type._to_string();
}

struct object
{
	// Use small_vector because most lang sequences will have very limited number of elements -
	// the longest sequences are color (R, G, B, + optonal A) and arrays of strings for string-based
	// conditions like BaseType.
	// This container optimizes storage so that only long arrays of strings will allocate.
	using container_type = boost::container::small_vector<single_object, 4>;

	container_type values;
	position_tag origin;
};

bool operator==(const object& lhs, const object& rhs) noexcept;

inline
bool operator!=(const object& lhs, const object& rhs) noexcept { return !(lhs == rhs); }

namespace detail {
	template <typename T> [[nodiscard]] constexpr
	object_type object_type_of_impl() noexcept
	{
		static_assert(sizeof(T) == 0, "missing implementation for this type");
		return object_type::none; // return statement only to silence compiler/editors
	}

	template <> constexpr
	object_type object_type_of_impl<none>() noexcept { return object_type::none; }
	template <> constexpr
	object_type object_type_of_impl<temp>() noexcept { return object_type::temp; }
	template <> constexpr
	object_type object_type_of_impl<boolean>() noexcept { return object_type::boolean; }
	template <> constexpr
	object_type object_type_of_impl<fractional>() noexcept { return object_type::fractional; }
	template <> constexpr
	object_type object_type_of_impl<integer>() noexcept { return object_type::integer; }
	template <> constexpr
	object_type object_type_of_impl<socket_spec>() noexcept { return object_type::socket_spec; }
	template <> constexpr
	object_type object_type_of_impl<influence>() noexcept { return object_type::influence; }
	template <> constexpr
	object_type object_type_of_impl<rarity>() noexcept { return object_type::rarity; }
	template <> constexpr
	object_type object_type_of_impl<shape>() noexcept { return object_type::shape; }
	template <> constexpr
	object_type object_type_of_impl<suit>() noexcept { return object_type::suit; }
	template <> constexpr
	object_type object_type_of_impl<shaper_voice_line>() noexcept { return object_type::shaper_voice_line; }
	template <> constexpr
	object_type object_type_of_impl<gem_quality_type>() noexcept { return object_type::gem_quality_type; }
	template <> constexpr
	object_type object_type_of_impl<string>() noexcept { return object_type::string; }
}

template <typename T> [[nodiscard]] constexpr
object_type object_type_of() noexcept
{
	static_assert(
		traits::is_variant_alternative_v<T, object_variant>,
		"T must be one of object type alternatives");

	return detail::object_type_of_impl<T>();
}

}
