#pragma once

#include "fs/lang/types.hpp"
#include "fs/utility/type_list.hpp"

namespace fs::lang
{

template <typename... Ts>
struct promotion_list
{
	using types = utility::type_list<Ts...>;
};

template <typename>
struct allowed_promotions : promotion_list<> {}; // by default no promotions are allowed

template <>
struct allowed_promotions<floating_point> : promotion_list<integer> {};

template <>
struct allowed_promotions<level> : promotion_list<integer> {};

template <>
struct allowed_promotions<quality> : promotion_list<integer> {};

template <>
struct allowed_promotions<font_size> : promotion_list<integer> {};

template <>
struct allowed_promotions<sound_id> : promotion_list<integer> {};

template <>
struct allowed_promotions<volume> : promotion_list<integer> {};

template <>
struct allowed_promotions<path> : promotion_list<string> {};

template <>
struct allowed_promotions<beam_effect> : promotion_list<suit> {};

template <> // note: the implementation does not allow to chain promotions (hence there both sound_id and integer and also path and string)
struct allowed_promotions<alert_sound> : promotion_list<sound_id, integer, path, string> {};

}
