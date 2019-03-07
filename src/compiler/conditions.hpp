#pragma once

#include "parser/ast.hpp"
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include "lang/condition_set.hpp"
#include "lang/constants_map.hpp"
#include "itemdata/types.hpp"

#include <optional>

namespace fs::compiler
{

[[nodiscard]]
std::optional<error::error_variant> add_conditions(
	const std::vector<parser::ast::condition>& conditions,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::condition_set& condition_set);

[[nodiscard]]
std::optional<error::error_variant> add_comparison_condition(
	const parser::ast::comparison_condition& condition,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::condition_set& condition_set);

[[nodiscard]]
std::optional<error::error_variant> add_string_condition(
	const parser::ast::string_condition& condition,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_strings_condition(
	std::vector<std::string> strings,
	lang::position_tag condition_origin,
	std::shared_ptr<std::vector<std::string>>& target);

[[nodiscard]]
std::optional<error::error_variant> add_boolean_condition(
	const parser::ast::boolean_condition& condition,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_boolean_condition(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	std::optional<lang::boolean>& target);

[[nodiscard]]
std::optional<error::error_variant> add_socket_group_condition(
	const parser::ast::socket_group_condition& condition,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_socket_group_condition(
	lang::socket_group socket_group,
	lang::position_tag condition_origin,
	std::optional<lang::socket_group>& target);

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
			if (target.is_exact())
				return error::exact_comparison_redefinition{condition_origin};

			if (!target.includes(value))
				return error::exact_comparison_outside_parent_range{condition_origin};

			target.set_exact(value);
			return std::nullopt;
		}
		case lang::comparison_type::less:
		{
			if (target.upper_bound.has_value())
				return error::upper_bound_redefinition{condition_origin};

			target.set_upper_bound(value, false);
			return std::nullopt;
		}
		case lang::comparison_type::less_equal:
		{
			if (target.upper_bound.has_value())
				return error::upper_bound_redefinition{condition_origin};

			target.set_upper_bound(value, true);
			return std::nullopt;
		}
		case lang::comparison_type::greater:
		{
			if (target.lower_bound.has_value())
				return error::lower_bound_redefinition{condition_origin};

			target.set_lower_bound(value, false);
			return std::nullopt;
		}
		case lang::comparison_type::greater_equal:
		{
			if (target.lower_bound.has_value())
				return error::lower_bound_redefinition{condition_origin};

			target.set_lower_bound(value, true);
			return std::nullopt;
		}
		default:
		{
			return error::internal_compiler_error_during_range_evaluation{condition_origin};
		}
	}
}

}
