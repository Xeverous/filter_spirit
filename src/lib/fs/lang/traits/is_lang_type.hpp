#pragma once

#include "fs/lang/types.hpp"

#include <type_traits>

namespace fs::lang::traits
{

    template <typename>
    struct is_lang_type : std::false_type {};

    template <>
    struct is_lang_type<boolean> : std::true_type {};

    template <>
    struct is_lang_type<floating_point> : std::true_type {};

    template <>
    struct is_lang_type<integer> : std::true_type {};

    template <>
    struct is_lang_type<level> : std::true_type {};

    template <>
    struct is_lang_type<font_size> : std::true_type {};

    template <>
    struct is_lang_type<sound_id> : std::true_type {};

    template <>
    struct is_lang_type<volume> : std::true_type {};

    template <>
    struct is_lang_type<socket_group> : std::true_type {};

    template <>
    struct is_lang_type<rarity> : std::true_type {};

    template <>
    struct is_lang_type<shape> : std::true_type {};

    template <>
    struct is_lang_type<suit> : std::true_type {};

    template <>
    struct is_lang_type<color> : std::true_type {};

    template <>
    struct is_lang_type<minimap_icon> : std::true_type {};

    template <>
    struct is_lang_type<beam_effect> : std::true_type {};

    template <>
    struct is_lang_type<string> : std::true_type {};

    template <>
    struct is_lang_type<path> : std::true_type {};

    template <>
    struct is_lang_type<built_in_alert_sound> : std::true_type {};

    template <>
    struct is_lang_type<custom_alert_sound> : std::true_type {};

    template <>
    struct is_lang_type<alert_sound> : std::true_type {};

    template <>
    struct is_lang_type<array_object> : std::true_type {};

    template <typename T>
    constexpr bool is_lang_type_v = is_lang_type<T>::value;

}
