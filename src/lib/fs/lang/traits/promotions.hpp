#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/object.hpp>
#include <fs/utility/type_list.hpp>

/*
 * NOTE: not C++ traits but traits for filter language types
 */
namespace fs::lang::traits
{

/*
 * specify a list of types that the destination type can be promoted from
 *
 * - promotions are backed by and only by 1-argument constructors
 * - promotions do not chain (only 1 may happen at any evaluation)
 */
template <typename... Ts>
struct promotion_list
{
	using types = utility::type_list<Ts...>;
};

using empty_promotion_list = promotion_list<>;

/*
 * specify a list of types that the destination type can be constructod from
 *
 * - requirement: std::is_constructible_v<DestinationType, Ts...>
 */
template <typename... Ts>
struct constructor_argument_list
{
	using types = utility::type_list<Ts...>;
	static constexpr auto arity = sizeof...(Ts);
};

/*
 * specify a list of constructors that the destination type can be promoted from
 *
 * - constructros are attempted in order of their listing, first match wins
 * - error only if all attempts failed
 */
template <typename... ConstructorArgumentLists>
struct constructor_list
{
	using constructors = utility::type_list<ConstructorArgumentLists...>;
};

using empty_constructor_list = constructor_list<>;

template <typename AllowedContructors, typename AllowedPromotions>
struct type_traits_base
{
	using allowed_constructors = AllowedContructors;
	using allowed_promotions = AllowedPromotions;
};

// by default no constructors and no promotions
template <typename T>
struct type_traits : type_traits_base<empty_constructor_list, empty_promotion_list> {};

template <>
struct type_traits<floating_point> : type_traits_base <
	empty_constructor_list,
	promotion_list<integer>> {};

template <>
struct type_traits<level> : type_traits_base <
	constructor_list <
		constructor_argument_list<integer>
	>,
	promotion_list<integer>> {};

template <>
struct type_traits<font_size> : type_traits_base <
	constructor_list <
		constructor_argument_list<integer>
	>,
	promotion_list<integer>> {};

template <>
struct type_traits<sound_id> : type_traits_base <
	constructor_list <
		constructor_argument_list<integer>
	>,
	promotion_list<integer>> {};

template <>
struct type_traits<volume> : type_traits_base <
	constructor_list <
		constructor_argument_list<integer>
	>,
	promotion_list<integer>> {};

template <>
struct type_traits<socket_group> : type_traits_base <
	constructor_list <
		constructor_argument_list<string>
	>,
	promotion_list<string>> {};

template <>
struct type_traits<color> : type_traits_base <
	constructor_list <
		constructor_argument_list<integer, integer, integer>,
		constructor_argument_list<integer, integer, integer, integer>
	>,
	empty_promotion_list> {};

template <>
struct type_traits<minimap_icon> : type_traits_base <
	constructor_list <
		constructor_argument_list<integer, suit, shape>
	>,
	empty_promotion_list> {};

template <>
struct type_traits<beam_effect> : type_traits_base <
	constructor_list <
		constructor_argument_list<suit>,
		constructor_argument_list<suit, boolean>
	>,
	promotion_list<suit>> {};

template <>
struct type_traits<path> : type_traits_base <
	constructor_list <
		constructor_argument_list<string>
	>,
	promotion_list<string>> {};

template <>
struct type_traits<built_in_alert_sound> : type_traits_base <
	constructor_list <
		constructor_argument_list<sound_id>,
		constructor_argument_list<integer>,
		constructor_argument_list<sound_id, volume>,
		constructor_argument_list<sound_id, boolean>,
		constructor_argument_list<sound_id, volume, boolean>
	>,
	promotion_list<sound_id, integer>> {};

template <>
struct type_traits<custom_alert_sound> : type_traits_base <
	constructor_list <
		constructor_argument_list<path>,
		constructor_argument_list<string>
	>,
	promotion_list<path, string>> {};

template <>
struct type_traits<alert_sound> : type_traits_base <
	constructor_list <
		constructor_argument_list<built_in_alert_sound>,
		constructor_argument_list<sound_id>,
		constructor_argument_list<integer>,
		constructor_argument_list<sound_id, volume>,
		constructor_argument_list<sound_id, boolean>,
		constructor_argument_list<sound_id, volume, boolean>,
		constructor_argument_list<custom_alert_sound>,
		constructor_argument_list<path>,
		constructor_argument_list<string>
	>,
	promotion_list<built_in_alert_sound, custom_alert_sound, sound_id, integer, path, string>> {};

}
