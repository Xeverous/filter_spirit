#pragma once

#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/get_value_as.hpp>

#include <utility>

namespace fs::compiler::detail
{

template <typename T, bool AllowPromotions = true>
[[nodiscard]] std::variant<T, compile_error>
evaluate_as(
	const parser::ast::value_expression& expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::variant<lang::object, compile_error> object_or_error = evaluate_value_expression(expression, symbols, item_price_data);

	if (std::holds_alternative<compile_error>(object_or_error))
		return std::get<compile_error>(std::move(object_or_error));

	auto& object = std::get<lang::object>(object_or_error);
	std::variant<T, compile_error> value_or_error = get_value_as<T, AllowPromotions>(object);

	if (std::holds_alternative<compile_error>(value_or_error))
		return std::get<compile_error>(std::move(value_or_error));
	else
		return std::get<T>(std::move(value_or_error));
}

}
