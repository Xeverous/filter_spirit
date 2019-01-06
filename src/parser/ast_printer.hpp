#pragma once
#include "ast_adapted.hpp"
#include "utility/type_traits.hpp"
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <iostream>

// workaround for Eclipse CDT not being able (yet) to
// parse "if constexpr" - define a macro that evaluates to
// nothing for Eclipse but to constexpr for the compiler
// https://stackoverflow.com/questions/41386945
#ifdef __CDT_PARSER__
#  define ECLIPSE_CONSTEXPR
#else
#  define ECLIPSE_CONSTEXPR constexpr
#endif // __CDT_PARSER__


namespace fs::ast
{

// recursive printer for the whole AST
struct printer
{
	printer(int indent = 0) : indent(indent) {}

	template <typename T>
	std::enable_if_t<fs::traits::is_iterable_v<T>>
	operator()(const T& ast) const
	{
		tab(indent);
		std::cout << "{\n";
		for (const auto& elem : ast)
		{
			printer(indent + 1)(elem);
			std::cout << ",\n";
		}
		tab(indent);
		std::cout << "}\n";
	}

	template <typename T>
	std::enable_if_t<std::is_integral_v<T>>
	operator()(const T& ast) const
	{
		tab(indent);
		std::cout << ast << ", ";
	}

	template <typename T>
	std::enable_if_t<fs::traits::has_pointer_semantics_v<T>>
	operator()(const T& obj) const
	{
		tab(indent);
		if (obj)
			printer(indent + 1)(*obj);
		else
			std::cout << "(empty)";
	}

	template <typename T>
	std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
	operator()(const T& seq) const
	{
		tab(indent);
		std::cout << "{";
		boost::fusion::for_each(
			seq,
			[this](const auto& arg) {
				printer(indent + 1)(arg);
				std::cout << ", ";
			}
		);
		std::cout << "}";
	}

	template <typename... T>
	void operator()(const boost::spirit::x3::variant<T...>& v) const
	{
		boost::apply_visitor(printer(indent + 1), v);
	}

	// print strings as strings, do not split them as a sequence of characters
	void operator()(const std::string& text) const
	{
		tab(indent);
		std::cout << text << "* ";
	}

	void tab(int spaces) const
	{
		for (int i = 0; i < spaces; ++i)
			std::cout << "  ";
	}

	int indent;
};

}
