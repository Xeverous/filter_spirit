#pragma once

#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <type_traits>
#include <iosfwd>

namespace fs::traits
{

template <typename, typename = void>
struct has_begin_end : std::false_type {};

template <typename T>
struct has_begin_end<T, std::void_t<
	decltype(std::declval<T>().begin()),
	decltype(std::declval<T>().end())
>> : std::true_type {};

template <typename T>
constexpr bool has_begin_end_v = has_begin_end<T>::value;



template <typename, typename = void>
struct has_container_member_types : std::false_type {};

template <typename T>
struct has_container_member_types<T, std::void_t<
	typename T::value_type,
	typename T::reference,
	typename T::const_reference,
	typename T::iterator,
	typename T::const_iterator,
	typename T::difference_type,
	typename T::size_type
>> : std::true_type {};

template <typename T>
constexpr bool has_container_member_types_v = has_container_member_types<T>::value;


template <typename T>
constexpr bool is_iterable_v = has_begin_end_v<T>;

template <typename T>
constexpr bool is_container_v = has_begin_end_v<T> && has_container_member_types_v<T>;


template <typename T>
using dereference_type_t = std::remove_cv_t<std::remove_reference_t<decltype(*std::declval<T>())>>;

template <typename T, typename = void>
struct has_pointer_semantics : std::false_type {};

template <typename T, typename = void>
struct has_pointer_semantics_impl : std::false_type {};

template <typename T>
struct has_pointer_semantics_impl<T, std::void_t<
	decltype(std::declval<T>()->~dereference_type_t<T>())
>> : std::true_type {};

template <typename T>
struct has_pointer_semantics<T, std::void_t<
	decltype(*std::declval<T>())
>> : has_pointer_semantics_impl<T> {};

template <typename T>
constexpr bool has_pointer_semantics_v = has_pointer_semantics<T>::value;


template <typename, typename = void>
struct is_printable : std::false_type {};

template <typename T>
struct is_printable<T, std::void_t<
	decltype(std::declval<std::ostream>() << std::declval<T>())
>> : std::true_type {};

template <typename T>
constexpr bool is_printable_v = is_printable<T>::value;

}
