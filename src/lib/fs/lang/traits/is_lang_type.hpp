#pragma once

#include <fs/lang/object.hpp>
#include <fs/utility/type_traits.hpp>

namespace fs::lang::traits
{

    template <typename T>
    struct is_lang_type
    {
	    static constexpr bool value = fs::traits::is_variant_alternative_v<T, lang::object_variant>;
    };

    template <typename T>
    constexpr bool is_lang_type_v = is_lang_type<T>::value;

}
