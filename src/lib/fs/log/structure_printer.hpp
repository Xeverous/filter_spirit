#pragma once

#include <fs/utility/type_traits.hpp>

#include <boost/fusion/include/for_each.hpp>
#include <boost/type_index.hpp>

#include <iostream>

namespace fs::log
{

/**
 * @class recursive printer for any nested structure
 *
 * @details this class can require non-transitive dependencies, eg if you want to print
 * Boost's Fusion struct, you need to include adaptation header before using this class
 */
struct structure_printer
{
	structure_printer(int indent = 0) : indent(indent) {}

	// required by boost's Visitor concept
	using result_type = void;

	template <typename T>
	std::enable_if_t<fs::traits::is_iterable_v<T>>
	operator()(const T& ast) const
	{
		tab(indent);
		std::cout << boost::typeindex::type_id<T>().pretty_name() << " [\n";
		for (const auto& elem : ast)
		{
			structure_printer(indent + 1)(elem);
		}
		tab(indent);
		std::cout << "]\n";
	}

	template <typename T>
	std::enable_if_t<std::is_arithmetic_v<T>>
	operator()(const T& arithmetic) const
	{
		tab(indent);
		std::cout << boost::typeindex::type_id<T>().pretty_name() << " { " << arithmetic << " }\n";
	}

	template <typename T>
	std::enable_if_t<std::is_empty_v<T> || traits::has_void_get_value_v<T>>
	operator()(const T&) const
	{
		tab(indent);
		std::cout << boost::typeindex::type_id<T>().pretty_name() << "\n";
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
			std::cout << boost::typeindex::type_id<T>().pretty_name() << "{ (empty) }\n";
		}
	}

	template <typename T>
	std::enable_if_t<boost::fusion::traits::is_sequence<T>::value>
	operator()(const T& seq) const
	{
		tab(indent);
		std::cout << boost::typeindex::type_id<T>().pretty_name() << " {\n";
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
	std::enable_if_t<traits::has_non_void_get_value_v<T>>
	operator()(const T& obj) const
	{
		tab(indent);
		std::cout << boost::typeindex::type_id<T>().pretty_name() << " {\n";
		structure_printer(indent + 1)(obj.get_value());
		tab(indent);
		std::cout << "}\n";
	}

	template <typename T>
	std::enable_if_t<std::is_enum_v<T>>
	operator()(T value) const
	{
		tab(indent);
		std::cout << "enum " << boost::typeindex::type_id<T>().pretty_name()
			<< " { " << static_cast<int>(value) << " }\n";
	}

	template <typename... T>
	void operator()(const boost::spirit::x3::variant<T...>& v) const
	{
		// do not indent variants - they hold 1 element only
		boost::apply_visitor(structure_printer(indent), v);
	}

	template <typename T>
	void operator()(const boost::spirit::x3::forward_ast<T>& ast) const
	{
		// do not remove extra () - vexing parse
		(structure_printer(indent))(ast.get());
	}

	// print strings as strings, do not split them as a sequence of characters
	void operator()(const std::string& text) const
	{
		tab(indent);
		std::cout << text << '\n';
	}

	void operator()(std::string_view text) const
	{
		tab(indent);
		std::cout << text << '\n';
	}

	void operator()(bool b) const
	{
		tab(indent);
		std::cout << "bool { ";
		if (b)
			std::cout << "true";
		else
			std::cout << "false";
		std::cout << " }\n";
	}

	// lowest priority overload for unmatched Ts
	template <typename T = void>
	void operator()(...) const
	{
		static_assert(sizeof(T) == 0, "Missing overload for some T or missing include for fusion types adaptations");
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
