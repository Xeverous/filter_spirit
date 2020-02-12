#pragma once

#include <fs/parser/ast.hpp>
#include <fs/lang/object.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/item_price_data.hpp>
#include <fs/lang/traits/promotions.hpp>
#include <fs/compiler/error.hpp>
#include <fs/compiler/detail/evaluate_as.hpp>
#include <fs/compiler/detail/type_constructors.hpp>
#include <fs/compiler/detail/determine_types_of.hpp>

#include <array>
#include <cassert>
#include <variant>
#include <memory>

namespace fs::compiler::detail
{

namespace impl
{
	// helper for get_expression_list_positions
	template <std::size_t... I> [[nodiscard]]
	auto make_array_from_expressions(
		const parser::ast::sf::value_expression_list& expressions,
		std::index_sequence<I...>)
	{
		return std::array<lang::position_tag, sizeof...(I)>{
			parser::get_position_info(expressions[I])...};
	}

	// helper for calling type_constructor
	template <std::size_t N> [[nodiscard]]
	std::array<lang::position_tag, N> get_expression_list_positions(
		const parser::ast::sf::value_expression_list& expressions)
	{
		assert(expressions.size() == N);
		return make_array_from_expressions(expressions, std::make_index_sequence<N>{});
	}

	// base case: all arguments unpacked
	// proceed to call constructor
	template <typename T, typename... Args> [[nodiscard]]
	std::variant<T, compile_error> unpack_args_and_call_constructor(
		const parser::ast::sf::value_expression_list& arguments,
		const lang::symbol_table& /* symbols */,
		const lang::item_price_data& /* item_price_data */,
		lang::traits::constructor_argument_list<>,
		Args&&... args)
	{
		return type_constructor<T>::call(
			parser::get_position_info(arguments),
			get_expression_list_positions<sizeof...(Args)>(arguments),
			std::forward<Args>(args)...);
	}

	// evaluate next argument
	// fail: return error
	// success: recursively call self to evaluate remaining arguments
	template <
		typename T,
		typename ConstructorArgType,
		typename... OtherConstructorArgTypes,
		typename... Args
	> [[nodiscard]]
	std::variant<T, compile_error> unpack_args_and_call_constructor(
		const parser::ast::sf::value_expression_list& arguments,
		const lang::symbol_table& symbols,
		const lang::item_price_data& item_price_data,
		lang::traits::constructor_argument_list<ConstructorArgType, OtherConstructorArgTypes...>,
		Args&&... args)
	{
		constexpr auto index = sizeof...(Args);
		std::variant<ConstructorArgType, compile_error> arg_or_error =
			evaluate_as<ConstructorArgType>(arguments[index], symbols, item_price_data);

		if (std::holds_alternative<compile_error>(arg_or_error))
			return std::get<compile_error>(std::move(arg_or_error));

		return unpack_args_and_call_constructor<T>(
			arguments,
			symbols,
			item_price_data,
			lang::traits::constructor_argument_list<OtherConstructorArgTypes...>{},
			std::forward<Args>(args)...,
			std::get<ConstructorArgType>(std::move(arg_or_error)));
	}

	// check if arguments amount matches
	// if so, unpack arguments and call constructor
	// if not, return errors::invalid_amount_of_arguments
	template <typename T, typename... ConstructorArgTypes> [[nodiscard]]
	std::variant<T, compile_error> construct_check_arguments_amount(
		const parser::ast::sf::value_expression_list& arguments,
		const lang::symbol_table& symbols,
		const lang::item_price_data& item_price_data,
		lang::traits::constructor_argument_list<ConstructorArgTypes...>)
	{
		const auto expected_arguments_count = sizeof...(ConstructorArgTypes);
		const auto actual_arguments_count = arguments.size();
		if (actual_arguments_count != expected_arguments_count) {
			return errors::invalid_amount_of_arguments{
				static_cast<int>(expected_arguments_count),
				static_cast<int>(actual_arguments_count),
				parser::get_position_info(arguments)};
		}

		return unpack_args_and_call_constructor<T>(
			arguments,
			symbols,
			item_price_data,
			lang::traits::constructor_argument_list<ConstructorArgTypes...>{});
	}

	// helper for append_function_call_errors()
	template <typename... Types>
	std::vector<lang::object_type> argument_types_to_vector(
		utility::type_list<Types...>)
	{
		std::vector<lang::object_type> result;
		result.reserve(sizeof...(Types));
		(result.push_back(lang::type_to_enum<Types>()), ...);
		return result;
	}

	// base case
	void append_function_call_errors(
		std::vector<errors::unmatched_function_call>&,
		lang::traits::constructor_list<>)
	{
	}

	// helper for base case of construct_attempt()
	template <
		typename FirstFailedConstructor,
		typename... OtherFailedConstructors,
		typename... Errors>
	void append_function_call_errors(
		std::vector<errors::unmatched_function_call>& v,
		lang::traits::constructor_list<FirstFailedConstructor, OtherFailedConstructors...>,
		compile_error&& error,
		Errors&&... remaining_errors)
	{
		static_assert(
			sizeof...(Errors) == sizeof...(OtherFailedConstructors),
			"logic error in code: some error objects are lost or duplicated");

		v.push_back(errors::unmatched_function_call{
			argument_types_to_vector(typename FirstFailedConstructor::types{}),
			std::forward<compile_error>(error)});
		append_function_call_errors(
			v,
			lang::traits::constructor_list<OtherFailedConstructors...>{},
			std::forward<Errors>(remaining_errors)...);
	}

	// base case
	// no more constructors available - return errors::no_matching_constructor_found
	// that contains error information about each failed attempt
	template <typename T, typename... FailedConstructors, typename... Errors> [[nodiscard]]
	std::variant<T, compile_error> construct_attempt(
		const parser::ast::sf::function_call& function_call,
		const lang::symbol_table& symbols,
		const lang::item_price_data& item_price_data,
		lang::traits::constructor_list<> /* ctors_to_attempt */,
		lang::traits::constructor_list<FailedConstructors...> /* failed_constructors */,
		Errors&&... errors_so_far)
	{
		static_assert(
			sizeof...(Errors) == sizeof...(FailedConstructors),
			"logic error in code: some error objects are lost or duplicated");
		/*
		 * if there is only 1 constructor don't report "no matching ctor found"
		 * as there clearly is only 1 candidate - report "failed ctor" instead
		 */
		if constexpr (sizeof...(FailedConstructors) == 1)
		{
			static_assert(sizeof...(Errors) == 1, "logic flaw: see comment above");
			// return compile_error(std::forward<Errors>(errors_so_far)...);
			return errors::failed_constructor_call{
				lang::type_to_enum<T>(),
				// we have to expand parameter packs with ... but we assert that their size is 1
				argument_types_to_vector(typename FailedConstructors::types{}...),
				parser::get_position_info(function_call),
				std::make_unique<compile_error>(std::forward<Errors>(errors_so_far)...)};
		}
		else
		{
			std::vector<errors::unmatched_function_call> errors;
			errors.reserve(sizeof...(Errors));
			append_function_call_errors(
				errors,
				lang::traits::constructor_list<FailedConstructors...>{},
				std::forward<Errors>(errors_so_far)...);

			return errors::no_matching_constructor_found{
				lang::type_to_enum<T>(),
				determine_types_of(function_call.arguments, symbols, item_price_data),
				parser::get_position_info(function_call),
				std::move(errors)};
		}
	}

	// try first constructor in the list
	// if failed - pass the error and recursively try next ones
	template <
		typename T,
		typename ConstructorArgumentList,
		typename... OtherConstructorArgumentLists,
		typename... FailedConstructors,
		typename... Errors // = compile_error; we template it only for perfect forwarding and wait for homogenous packs feature
	> [[nodiscard]]
	std::variant<T, compile_error> construct_attempt(
		const parser::ast::sf::function_call& function_call,
		const lang::symbol_table& symbols,
		const lang::item_price_data& item_price_data,
		lang::traits::constructor_list<ConstructorArgumentList, OtherConstructorArgumentLists...> /* ctors_to_attempt */,
		lang::traits::constructor_list<FailedConstructors...> /* failed_constructors */,
		Errors&&... errors_so_far)
	{
		std::variant<T, compile_error> result = construct_check_arguments_amount<T>(
			function_call.arguments, symbols, item_price_data, ConstructorArgumentList{});

		if (std::holds_alternative<compile_error>(result)) {
			return construct_attempt<T>(
				function_call,
				symbols,
				item_price_data,
				lang::traits::constructor_list<OtherConstructorArgumentLists...>{},
				lang::traits::constructor_list<FailedConstructors..., ConstructorArgumentList>{},
				std::forward<Errors>(errors_so_far)...,
				std::get<compile_error>(std::move(result)));
		}

		return std::get<T>(std::move(result));
	}

} // namespace impl

template <typename T> [[nodiscard]]
std::variant<T, compile_error> construct(
	const parser::ast::sf::function_call& function_call,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	using constructors = typename lang::traits::type_traits<T>::allowed_constructors;
	return impl::construct_attempt<T>(function_call, symbols, item_price_data, constructors{}, lang::traits::constructor_list<>{});
}

}
