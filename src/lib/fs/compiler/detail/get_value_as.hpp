#pragma once

#include "fs/lang/types.hpp"
#include "fs/lang/traits/promotions.hpp"
#include "fs/lang/traits/is_lang_type.hpp"
#include "fs/utility/type_list.hpp"
#include "fs/compiler/error.hpp"
#include "fs/compiler/detail/type_constructors.hpp"

#include <array>
#include <variant>
#include <type_traits>

namespace fs::compiler::detail
{

namespace impl
{

	template <typename T>
	std::variant<T, compile_error> attempt_to_promote(const lang::object& object, lang::traits::promotion_list<>)
	{
		return errors::type_mismatch{
			lang::type_to_enum<T>(),
			lang::type_of_object(object.value),
			object.value_origin};
	}

	template <typename T, typename P, typename... Ps>
	std::variant<T, compile_error> attempt_to_promote(const lang::object& object, lang::traits::promotion_list<P, Ps...>)
	{
		if (std::holds_alternative<P>(object.value))
			return type_constructor<T>::call(
				object.value_origin,
				std::array<lang::position_tag, 1>{object.value_origin},
				std::get<P>(object.value));
		else
			return attempt_to_promote<T>(object, lang::traits::promotion_list<Ps...>{});
	}

	template <typename T>
	std::variant<T, compile_error> get_non_array_value_as(const lang::object& object)
	{
		if (std::holds_alternative<T>(object.value))
			return std::get<T>(object.value);
		else
			return attempt_to_promote<T>(object, typename lang::traits::type_traits<T>::allowed_promotions{});
	}

}

template <typename T, bool AllowPromotions = true>
std::variant<T, compile_error> get_value_as(const lang::object& object)
{
	static_assert(lang::traits::is_lang_type_v<T>, "T should be one of FS language types");

	if constexpr (AllowPromotions)
	{
		if constexpr (std::is_same_v<T, lang::array_object>)
		{
			if (!object.is_array())
				return object.promote_to_array();

			return std::get<lang::array_object>(object.value);
		}
		else
		{
			return impl::get_non_array_value_as<T>(object);
		}
	}
	else
	{
		if (std::holds_alternative<T>(object.value))
			return std::get<T>(object.value);
		else
			return errors::type_mismatch{
				lang::type_to_enum<T>(),
				lang::type_of_object(object.value),
				object.value_origin};
	}
}

}
