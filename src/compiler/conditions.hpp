#pragma once
#include "parser/ast.hpp"
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include "lang/condition_set.hpp"
#include "lang/constants_map.hpp"
#include <optional>

namespace fs::compiler
{

[[nodiscard]]
std::optional<error::error_variant> add_conditions(
	const std::vector<parser::ast::condition>& conditions,
	const lang::constants_map& map,
	lang::condition_set& condition_set);

[[nodiscard]]
std::optional<error::error_variant> add_comparison_condition(
	const parser::ast::comparison_condition& condition,
	const lang::constants_map& map,
	lang::condition_set& condition_set);

[[nodiscard]]
std::optional<error::error_variant> add_string_condition(
	const parser::ast::string_condition& condition,
	const lang::constants_map& map,
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
	lang::condition_set& condition_set);

// helper for the function above
[[nodiscard]]
std::optional<error::error_variant> add_socket_group_condition(
	lang::socket_group socket_group,
	lang::position_tag condition_origin,
	std::optional<lang::socket_group>& target);

template <typename T>
std::optional<error::error_variant> add_range_condition(
	lang::range_condition<T> range_condition,
	lang::position_tag condition_origin,
	std::optional<lang::range_condition<T>>& target)
{
	if (target.has_value())
		return error::condition_redefinition{condition_origin}; // FIXME this is not fully correct

	// FIXME compare ranges
	return std::nullopt;
}

}
