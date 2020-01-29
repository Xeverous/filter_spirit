#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/compiler/detail/get_value_as.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/lang/item_price_data.hpp>
#include <fs/utility/type_traits.hpp>

#include <variant>

namespace fs::compiler::detail
{

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_value_expression(
	const parser::ast::sf::value_expression& value_expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data);

[[nodiscard]] inline lang::string
evaluate(parser::ast::rf::string_literal sl)
{
	return {static_cast<std::string>(sl)};
}

[[nodiscard]] inline lang::boolean
evaluate(parser::ast::rf::boolean_literal bl)
{
	return {bl.value};
}

[[nodiscard]] inline lang::integer
evaluate(parser::ast::rf::integer_literal il)
{
	return {il.value};
}

[[nodiscard]] inline lang::color
evaluate(parser::ast::rf::color_literal cl)
{
	if (cl.a.has_value()) {
		return lang::color(cl.r.value, cl.g.value, cl.b.value, (*cl.a).value);
	}
	else {
		return lang::color(cl.r.value, cl.g.value, cl.b.value);
	}
}

[[nodiscard]] inline lang::rarity
evaluate(parser::ast::rf::rarity_literal rl)
{
	return rl.value;
}

[[nodiscard]] inline lang::suit
evaluate(parser::ast::rf::suit_literal sl)
{
	return sl.value;
}

[[nodiscard]] inline lang::shape
evaluate(parser::ast::rf::shape_literal sl)
{
	return sl.value;
}

[[nodiscard]] inline lang::influence
evaluate(parser::ast::rf::influence_literal il)
{
	return il.value;
}

[[nodiscard]] std::variant<lang::minimap_icon, compile_error>
evaluate(parser::ast::rf::icon_literal il);

[[nodiscard]] std::variant<lang::socket_group, compile_error>
evaluate_as_socket_group(const std::string& str, lang::position_tag origin);

template <typename LangType, bool AllowPromotions = true>
[[nodiscard]] std::variant<LangType, compile_error>
evaluate_as(
	const parser::ast::sf::value_expression& expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::variant<lang::object, compile_error> object_or_error = evaluate_value_expression(expression, symbols, item_price_data);

	if (std::holds_alternative<compile_error>(object_or_error))
		return std::get<compile_error>(std::move(object_or_error));

	auto& object = std::get<lang::object>(object_or_error);
	return detail::get_value_as<LangType, AllowPromotions>(object);
}

template <typename LangType>
[[nodiscard]] std::variant<lang::object, compile_error>
generalize_type(
	std::variant<LangType, compile_error>&& lt_or_error,
	lang::position_tag origin)
{
	if (std::holds_alternative<compile_error>(lt_or_error))
		return std::get<compile_error>(std::move(lt_or_error));

	return lang::object{std::get<LangType>(lt_or_error), origin};
}

} // namespace fs::compiler::detail
