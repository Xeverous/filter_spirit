#pragma once
#include "parser/ast_adapted.hpp"
#include "utility/type_traits.hpp"
#include "utility/type_name.hpp"
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <iostream>

namespace fs::print
{

// recursive printer for any nested structure
struct structure_printer
{
	structure_printer(int indent = 0) : indent(indent) {}

	template <typename T>
	std::enable_if_t<fs::traits::is_iterable_v<T>>
	operator()(const T& ast) const
	{
		tab(indent);
		std::cout << fs::utility::type_name<T>().get() << " [\n";
		for (const auto& elem : ast)
		{
			structure_printer(indent + 1)(elem);
		}
		tab(indent);
		std::cout << "]\n";
	}

	template <typename T>
	std::enable_if_t<std::is_integral_v<T>>
	operator()(const T& integral) const
	{
		tab(indent);
		std::cout << integral;
		std::cout << "\n";
	}

	template <typename T>
	std::enable_if_t<fs::traits::has_pointer_semantics_v<T>>
	operator()(const T& obj) const
	{
		if (obj)
		{
			// do not indent optionals (they contain only 1 value)
			// do not remove extra () - vexing parse
			(structure_printer(indent))(*obj);
		}
		else
		{
			tab(indent);
			std::cout << "(empty)\n";
		}
	}

	template <typename T>
	std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
	operator()(const T& seq) const
	{
		tab(indent);
		std::cout << fs::utility::type_name<T>().get() << " {\n";
		boost::fusion::for_each(
			seq,
			[this](const auto& arg) {
				structure_printer(indent + 1)(arg);
			}
		);
		tab(indent);
		std::cout << "}\n";
	}

	template <typename T>
	std::enable_if_t<std::is_enum_v<T>>
	operator()(T value) const
	{
		tab(indent);
		std::cout << "enum: " << static_cast<int>(value) << "\n";
	}

	template <typename... T>
	void operator()(const boost::spirit::x3::variant<T...>& v) const
	{
		// do not indent variants - they hold 1 element only
		boost::apply_visitor(structure_printer(indent), v);
	}

	// print strings as strings, do not split them as a sequence of characters
	void operator()(const std::string& text) const
	{
		tab(indent);
		std::cout << text << "\n";
	}

	void operator()(std::string_view text) const
	{
		tab(indent);
		std::cout << text << "\n";
	}

	// lowest priority overload for unmatched Ts
	template <typename T = void>
	void operator()(...) const
	{
		static_assert(sizeof(T) == 0, "Missing overload for this T");
	}

	static
	void tab(int spaces)
	{
		for (int i = 0; i < spaces; ++i)
			std::cout << "  ";
	}

	const int indent;
};

}
