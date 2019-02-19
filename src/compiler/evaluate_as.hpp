#pragma once
#include "compiler/evaluate.hpp"
#include "compiler/get_value_as.hpp"

namespace fs::compiler
{

template <typename T>
std::variant<T, error::error_variant> evaluate_as(
	const parser::ast::value_expression& expression,
	const lang::constants_map& map)
{
	std::variant<lang::object, error::error_variant> object_or_error = evaluate_value_expression(expression, map);

	if (std::holds_alternative<error::error_variant>(object_or_error))
		return std::get<error::error_variant>(object_or_error);

	auto& object = std::get<lang::object>(object_or_error);
	std::variant<T, error::error_variant> value_or_error = get_value_as<T>(object);

	if (std::holds_alternative<error::error_variant>(value_or_error))
		return std::get<error::error_variant>(value_or_error);
	else
		return std::get<T>(value_or_error);
}

}
