#pragma once
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <type_traits>
#include <variant>
#include <iosfwd>

namespace fs::traits
{

namespace detail
{
	template <typename, typename = void>
	struct has_begin_end_impl : std::false_type {};

	template <typename T>
	struct has_begin_end_impl<
		T,
		std::void_t<
			decltype(std::declval<T>().begin()),
			decltype(std::declval<T>().end())>
	> : std::true_type {};



	template <typename, typename = void>
	struct has_container_member_types_impl : std::false_type {};

	template <typename T>
	struct has_container_member_types_impl<
		T,
		std::void_t<
			typename T::value_type,
			typename T::reference,
			typename T::const_reference,
			typename T::iterator,
			typename T::const_iterator,
			typename T::difference_type,
			typename T::size_type>
	> : std::true_type {};



	template <typename T>
	using dereference_type_t = std::remove_cv_t<std::remove_reference_t<decltype(*std::declval<T>())>>;

	template <typename, typename = void>
	struct has_pointer_semantics_impl : std::false_type {};

	template <typename, typename = void>
	struct is_dereferenceable : std::false_type {};

	template <typename T>
	struct is_dereferenceable<T, std::void_t<
		decltype(std::declval<T>()->~dereference_type_t<T>())
	>> : std::true_type {};

	template <typename T>
	struct has_pointer_semantics_impl<T, std::void_t<
		decltype(*std::declval<T>())
	>> : is_dereferenceable<T> {};



	template <typename, typename = void>
	struct is_printable_impl : std::false_type {};

	template <typename T>
	struct is_printable_impl<T, std::void_t<
		decltype(std::declval<std::ostream>() << std::declval<T>())
	>> : std::true_type {};



	template <typename... Ts>
	struct type_list {};

	template <typename... Ts>
	type_list<Ts...> list_of_variant_alternatives(std::variant<Ts...> v);

	template <typename T, typename... Ts>
	constexpr bool is_one_of(type_list<Ts...>)
	{
		return (std::is_same_v<T, Ts> || ...);
	}

	template <typename T, typename Variant>
	struct is_variant_alternative_impl
	{
		using types = decltype(list_of_variant_alternatives(std::declval<Variant>()));
		static constexpr bool value = is_one_of<T>(types{});
	};



	template <typename, typename = void>
	struct has_non_void_get_value_impl : std::false_type {};

	template <typename T>
	struct has_non_void_get_value_impl<T, std::void_t<
		decltype(std::declval<T>().get_value())
	>>
	{
		static constexpr bool value = !std::is_same_v<void, decltype(std::declval<T>().get_value())>;
	};

	template <typename, typename = void>
	struct has_void_get_value_impl : std::false_type {};

	template <typename T>
	struct has_void_get_value_impl<T, std::void_t<
		decltype(std::declval<T>().get_value())
	>>
	{
		static constexpr bool value = std::is_same_v<void, decltype(std::declval<T>().get_value())>;
	};
}

template <typename T, typename = void>
struct has_begin_end : detail::has_begin_end_impl<T> {};

template <typename T>
constexpr bool has_begin_end_v = has_begin_end<T>::value;



template <typename T, typename = void>
struct has_container_member_types : detail::has_container_member_types_impl<T> {};

template <typename T>
constexpr bool has_container_member_types_v = has_container_member_types<T>::value;

template <typename T>
constexpr bool is_iterable_v = has_begin_end_v<T>;

template <typename T>
constexpr bool is_container_v = has_begin_end_v<T> && has_container_member_types_v<T>;



template <typename T, typename = void>
struct has_pointer_semantics : detail::has_pointer_semantics_impl<T> {};

template <typename T>
constexpr bool has_pointer_semantics_v = has_pointer_semantics<T>::value;



template <typename T, typename = void>
struct is_printable : detail::is_printable_impl<T> {};

template <typename T>
constexpr bool is_printable_v = is_printable<T>::value;



template <typename...>
struct variant_size;

template <typename... Ts>
struct variant_size<std::variant<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename... Ts>
constexpr auto variant_size_v = variant_size<Ts...>::value;



template <typename T, typename Variant>
struct is_variant_alternative : detail::is_variant_alternative_impl<T, Variant> {};

template <typename T, typename Variant>
constexpr bool is_variant_alternative_v = is_variant_alternative<T, Variant>::value;



template <typename T, typename = void>
struct has_non_void_get_value : detail::has_non_void_get_value_impl<T> {};

template <typename T>
constexpr bool has_non_void_get_value_v = has_non_void_get_value<T>::value;

template <typename T, typename = void>
struct has_void_get_value : detail::has_void_get_value_impl<T> {};

template <typename T>
constexpr bool has_void_get_value_v = has_void_get_value<T>::value;

}
