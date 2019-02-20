#pragma once
#include "lang/types.hpp"
#include "compiler/error.hpp"
#include "utility/if_constexpr_workaround.hpp"
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

template <typename T, typename P, typename... Ps>
std::variant<T, error::error_variant> attempt_to_promote(
	const lang::single_object& single_object,
	lang::position_tag object_position,
	type_list<P, Ps...>)
{
	if (std::holds_alternative<P>(single_object))
		return T(std::get<P>(single_object));
	else
		return attempt_to_promote<T>(single_object, object_position, type_list<Ps...>{});
}

template <typename T>
std::variant<T, error::error_variant> attempt_to_promote(
	const lang::single_object& single_object,
	lang::position_tag object_position,
	type_list<>)
{
	return error::type_mismatch{
		lang::type_to_enum<T>(),
		lang::type_of_object(single_object),
		object_position};
}

template <typename T>
std::variant<T, error::error_variant> get_non_array_value_as(
	const lang::single_object& single_object,
	lang::position_tag object_position)
{
	if (std::holds_alternative<T>(single_object))
		return std::get<T>(single_object);
	else
		return attempt_to_promote<T>(single_object, object_position, typename allowed_promotions<T>::types{});
}

template <typename T>
std::variant<T, error::error_variant> get_non_array_value_as(const lang::object& object)
{
	if (object.is_array())
		return error::type_mismatch{
			lang::type_to_enum<T>(),
			type_of_object(object),
			object.value_origin};

	const auto& single_object = std::get<lang::single_object>(object.value);
	return get_non_array_value_as<T>(single_object, object.value_origin);
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
