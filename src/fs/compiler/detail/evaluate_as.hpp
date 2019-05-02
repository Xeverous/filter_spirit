#pragma once

#include "fs/compiler/detail/evaluate.hpp"
#include "fs/compiler/detail/get_value_as.hpp"

#include <utility>

namespace fs::compiler::detail
{

template <typename T>
std::variant<T, error::error_variant> evaluate_as(
	const parser::ast::value_expression& expression,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	std::variant<lang::object, error::error_variant> object_or_error = evaluate_value_expression(expression, map, item_price_data);

	if (std::holds_alternative<error::error_variant>(object_or_error))
		return std::get<error::error_variant>(object_or_error);

	auto& object = std::get<lang::object>(object_or_error);
	std::variant<T, error::error_variant> value_or_error = get_value_as<T>(object);

	if (std::holds_alternative<error::error_variant>(value_or_error))
		return std::get<error::error_variant>(value_or_error);
	else
		return std::get<T>(std::move(value_or_error));
}

}
