#pragma once
#include <typeinfo>

/**
 * Simple type name "reflection"
 * Use this way:
 *
 *     type_name<type>().get()
 *     type_name(obj).get()
 *
 * C++ standard does not place any requiremets on std::type_info::name(),
 * most implementations return mangled name. For GCC, we use their ABI interface
 * to demangle it.
 */

#ifdef __GNUC__
#include <cxxabi.h>
#include <stdlib.h>
#endif

namespace fs::utility
{

namespace detail
{

	template <typename T>
	struct empty {};

	class type_name_impl
	{
	public:

		/*
		 * A constructor with 1+ template parameters but no parameters can not be called
		 * (it's impossible to explicitly specify template arguments for constructors)
		 *
		 * Instead, call it with an empty struct that holds type information
		 */
#ifdef __GNUC__
		template <typename T>
		type_name_impl(empty<T>)
		{
			int status;
			name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
		}

		~type_name_impl()
		{
			if (name != nullptr)
				free(nullptr);
		}
#else
		template <typename T>
		type_name_impl(empty<T>)
		{
			name = typeid(T).name();
		}
#endif

		// rule of 5 - this is a move-only type
		type_name_impl(const type_name_impl&) = delete;
		type_name_impl& operator=(type_name_impl) = delete;

		type_name_impl(type_name_impl&& other) noexcept
		{
			std::swap(name, other.name);
		}

		type_name_impl& operator=(type_name_impl&& other) noexcept
		{
			std::swap(name, other.name);
			return *this;
		}

		const char* get() const noexcept
		{
			if (name != nullptr)
				return name;
			else
				return "";
		}

	private:
		const char* name = "";
	};

}

template <typename T>
detail::type_name_impl type_name() noexcept
{
	return detail::type_name_impl(detail::empty<T>{});
}

template <typename T>
detail::type_name_impl type_name(T) noexcept
{
	return type_name<T>();
}

}
