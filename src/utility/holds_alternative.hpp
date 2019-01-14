#pragma once

#include <boost/variant.hpp>

/*
 * boost::variant does not provide something like
 * std::holds_alternative, but it can be easily written
 *
 * boost::get() has multiple overloads, and when passed a pointer
 * it can return a (possibly null) pointer to requested type
 *
 * function intentionally in global scope for the purpose of ADL
 */
template <typename T, typename... Ts> constexpr
bool holds_alternative(const boost::variant<Ts...>& v) noexcept
{
	return boost::get<T>(&v) != nullptr;
}
