#pragma once

#include "fs/lang/types.hpp"
#include "fs/lang/type_promotions.hpp"
#include "fs/utility/type_list.hpp"
#include "fs/utility/if_constexpr_workaround.hpp"
#include "fs/compiler/error.hpp"

#include <variant>
#include <type_traits>

namespace fs::compiler::detail
{

template <typename T>
std::variant<T, error::error_variant> attempt_to_promote(const lang::object& object, utility::type_list<>)
{
	return error::type_mismatch{
		lang::type_to_enum<T>(),
		lang::type_of_object(object.value),
		object.value_origin};
}

template <typename T, typename P, typename... Ps>
std::variant<T, error::error_variant> attempt_to_promote(const lang::object& object, utility::type_list<P, Ps...>)
{
	static_assert(std::is_constructible_v<T, P>, "T must define a constructor that takes P");

	if (std::holds_alternative<P>(object.value))
		return T(std::get<P>(object.value));
	else
		return attempt_to_promote<T>(object, utility::type_list<Ps...>{});
}

template <typename T>
std::variant<T, error::error_variant> get_non_array_value_as(const lang::object& object)
{
	if (std::holds_alternative<T>(object.value))
		return std::get<T>(object.value);
	else
		return attempt_to_promote<T>(object, typename lang::allowed_promotions<T>::types{});
}

template <typename T>
std::variant<T, error::error_variant> get_value_as(const lang::object& object)
{
	if ECLIPSE_CONSTEXPR (std::is_same_v<T, lang::array_object>)
	{
		if (!object.is_array())
			return object.promote_to_array();

		return std::get<lang::array_object>(object.value);
	}
	else
	{
		return get_non_array_value_as<T>(object);
	}
}

}
