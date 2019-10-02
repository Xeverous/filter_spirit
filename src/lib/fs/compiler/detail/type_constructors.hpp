#pragma once

#include "fs/compiler/error.hpp"
#include "fs/parser/ast.hpp"
#include "fs/lang/types.hpp"

#include <array>
#include <type_traits>

namespace fs::compiler::detail
{

// default constructor for any language type: just forwards arguments
template <typename T>
struct type_constructor
{
	template <std::size_t N, typename... Args> [[nodiscard]] static
	std::variant<T, compile_error> call(
		lang::position_tag /* origin */,
		std::array<lang::position_tag, N> /* argument_origins */,
		Args&&... args)
	{
		static_assert(sizeof...(Args) == N, "logic flaw: there should be the same amount of origins as there are arguments");
		static_assert(std::is_constructible_v<T, Args...>, "T must define a constructor that takes Args...");
		return T(std::forward<Args>(args)...);
	}
};

template <>
struct type_constructor<lang::socket_group>
{
	[[nodiscard]] static
	std::variant<lang::socket_group, compile_error> call(
		lang::position_tag origin,
		std::array<lang::position_tag, 1> argument_origins,
		const lang::string& string)
	{
		if (string.value.empty())
			return errors::empty_socket_group{argument_origins[0]};

		lang::socket_group sg;
		for (char c : string.value)
		{
			if (c == 'R')
				++sg.r;
			else if (c == 'G')
				++sg.g;
			else if (c == 'B')
				++sg.b;
			else if (c == 'W')
				++sg.w;
			else
				return errors::illegal_characters_in_socket_group{
					parser::get_position_info(argument_origins[0])};
		}

		if (!sg.is_valid())
			return errors::invalid_socket_group{origin};

		return sg;
	}
};

template <>
struct type_constructor<lang::minimap_icon>
{
	[[nodiscard]] static
	std::variant<lang::minimap_icon, compile_error> call(
		lang::position_tag /* origin */,
		std::array<lang::position_tag, 3> argument_origins,
		lang::integer size,
		lang::suit suit,
		lang::shape shape)
	{
		if (size.value != 0 && size.value != 1 && size.value != 2)
			return errors::invalid_minimap_icon_size{size.value, argument_origins[0]};

		return lang::minimap_icon{size, suit, shape};
	}
};

}
