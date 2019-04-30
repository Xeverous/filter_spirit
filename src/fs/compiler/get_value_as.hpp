#pragma once
#include "fs/lang/types.hpp"
#include "fs/compiler/error.hpp"
#include "fs/utility/if_constexpr_workaround.hpp"
#include <variant>
#include <type_traits>

namespace fs::compiler
{

template <typename... Ts>
struct type_list {};

template <typename... Ts>
struct promotion_list
{
	using types = type_list<Ts...>;
};

template <typename>
struct allowed_promotions : promotion_list<> {}; // by default no promotions are allowed

template <>
struct allowed_promotions<lang::floating_point> : promotion_list<lang::integer> {};

template <>
struct allowed_promotions<lang::level> : promotion_list<lang::integer> {};

template <>
struct allowed_promotions<lang::quality> : promotion_list<lang::integer> {};

template <>
struct allowed_promotions<lang::font_size> : promotion_list<lang::integer> {};

template <>
struct allowed_promotions<lang::sound_id> : promotion_list<lang::integer> {};

template <>
struct allowed_promotions<lang::volume> : promotion_list<lang::integer> {};

template <>
struct allowed_promotions<lang::path> : promotion_list<lang::string> {};

template <>
struct allowed_promotions<lang::beam_effect> : promotion_list<lang::suit> {};

template <> // note: the implementation does not allow to chain promotions (hence there both sound_id and integer and also path and string)
struct allowed_promotions<lang::alert_sound> : promotion_list<lang::sound_id, lang::integer, lang::path, lang::string> {};

template <typename T, typename P, typename... Ps>
std::variant<T, error::error_variant> attempt_to_promote(const lang::object& object, type_list<P, Ps...>)
{
	static_assert(std::is_constructible_v<T, P>, "T must define a constructor that takes P");

	if (std::holds_alternative<P>(object.value))
		return T(std::get<P>(object.value));
	else
		return attempt_to_promote<T>(object, type_list<Ps...>{});
}

template <typename T>
std::variant<T, error::error_variant> attempt_to_promote(const lang::object& object, type_list<>)
{
	return error::type_mismatch{
		lang::type_to_enum<T>(),
		lang::type_of_object(object),
		object.value_origin};
}

template <typename T>
std::variant<T, error::error_variant> get_non_array_value_as(const lang::object& object)
{
	if (std::holds_alternative<T>(object.value))
		return std::get<T>(object.value);
	else
		return attempt_to_promote<T>(object, typename allowed_promotions<T>::types{});
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
