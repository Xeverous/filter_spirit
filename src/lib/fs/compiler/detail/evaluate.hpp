#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/symbol_table.hpp>

#include <optional>
#include <variant>
#include <utility>

namespace fs::compiler::detail
{

[[nodiscard]] std::variant<lang::color, compile_error>
make_color(
	std::pair<lang::integer, lang::position_tag> r,
	std::pair<lang::integer, lang::position_tag> g,
	std::pair<lang::integer, lang::position_tag> b,
	std::optional<std::pair<lang::integer, lang::position_tag>> a);

[[nodiscard]] std::variant<lang::socket_spec, compile_error>
make_socket_spec(
	const std::string& raw,
	lang::position_tag origin);

[[nodiscard]] inline std::variant<lang::minimap_icon, compile_error>
make_minimap_icon(
	lang::integer size,
	lang::position_tag size_origin,
	lang::suit suit,
	lang::shape shape)
{
	if (size.value != 0 && size.value != 1 && size.value != 2)
		return errors::invalid_integer_value{0, 2, size.value, size_origin};

	return lang::minimap_icon{size, suit, shape};
}

[[nodiscard]] std::variant<lang::builtin_alert_sound, compile_error>
make_builtin_alert_sound(
	bool positional,
	std::pair<lang::integer, lang::position_tag> sound_id,
	std::optional<std::pair<lang::integer, lang::position_tag>> volume);

[[nodiscard]] std::variant<lang::socket_spec, compile_error>
evaluate_socket_spec_literal(
	const parser::ast::common::socket_spec_literal& literal);

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_sequence(
	const parser::ast::sf::sequence& sequence,
	const lang::symbol_table& symbols,
	int min_allowed_elements = 1,
	std::optional<int> max_allowed_elements = std::nullopt);

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_value_expression(
	const parser::ast::sf::value_expression& value_expression,
	const lang::symbol_table& symbols);

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

[[nodiscard]] inline std::variant<lang::color, compile_error>
evaluate(parser::ast::rf::color_literal cl)
{
	std::optional<std::pair<lang::integer, lang::position_tag>> a;
	if (cl.a) {
		a = std::make_pair(evaluate(*cl.a), parser::position_tag_of(*cl.a));
	}

	return make_color(
		{evaluate(cl.r), parser::position_tag_of(cl.r)},
		{evaluate(cl.g), parser::position_tag_of(cl.g)},
		{evaluate(cl.b), parser::position_tag_of(cl.b)},
		a
	);
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

[[nodiscard]] inline std::variant<lang::minimap_icon, compile_error>
evaluate(parser::ast::rf::icon_literal il)
{
	return make_minimap_icon(
		detail::evaluate(il.size),
		parser::position_tag_of(il.size),
		detail::evaluate(il.suit),
		detail::evaluate(il.shape)
	);
}

template <typename T>
[[nodiscard]] std::variant<T, compile_error>
get_as(const lang::single_object& sobj)
{
	if (!std::holds_alternative<T>(sobj.value)) {
		return errors::type_mismatch{lang::object_type_of<T>(), sobj.type(), sobj.origin};
	}

	return std::get<T>(sobj.value);
}

[[nodiscard]] std::variant<lang::socket_spec, compile_error>
get_as_socket_spec(
	const lang::single_object& obj);

} // namespace fs::compiler::detail
