#pragma once

#include "fs/parser/ast.hpp"
#include "fs/compiler/error.hpp"
#include "fs/lang/types.hpp"
#include "fs/lang/condition_set.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/lang/item_price_data.hpp"

#include <optional>

namespace fs::compiler::detail
{

[[nodiscard]]
std::optional<error::error_variant> add_conditions(
	const std::vector<parser::ast::condition>& conditions,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set);

[[nodiscard]]
std::optional<error::error_variant> add_comparison_condition(
	const parser::ast::comparison_condition& condition,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set);

[[nodiscard]]
std::optional<error::error_variant> add_string_condition(
	const parser::ast::string_condition& condition,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_strings_condition(
	std::vector<std::string> strings,
	lang::position_tag condition_origin,
	lang::strings_condition& target);

[[nodiscard]]
std::optional<error::error_variant> add_boolean_condition(
	const parser::ast::boolean_condition& condition,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_boolean_condition(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	std::optional<lang::boolean_condition>& target);

[[nodiscard]]
std::optional<error::error_variant> add_socket_group_condition(
	const parser::ast::socket_group_condition& condition,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_socket_group_condition(
	lang::socket_group socket_group,
	lang::position_tag condition_origin,
	std::optional<lang::socket_group_condition>& target);

template <typename T>
std::optional<error::error_variant> add_range_condition(
	lang::comparison_type comparison_type,
	T value,
	lang::position_tag condition_origin,
	lang::range_condition<T>& target)
{
	switch (comparison_type)
	{
		case lang::comparison_type::equal:
		{
			if (target.lower_bound.has_value())
				return error::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			if (target.upper_bound.has_value())
				return error::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_exact(value, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::less:
		{
			if (target.upper_bound.has_value())
				return error::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_upper_bound(value, false, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::less_equal:
		{
			if (target.upper_bound.has_value())
				return error::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_upper_bound(value, true, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::greater:
		{
			if (target.lower_bound.has_value())
				return error::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			target.set_lower_bound(value, false, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::greater_equal:
		{
			if (target.lower_bound.has_value())
				return error::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			target.set_lower_bound(value, true, condition_origin);
			return std::nullopt;
		}
		default:
		{
			return error::internal_compiler_error_during_range_evaluation{condition_origin};
		}
	}
}

}
