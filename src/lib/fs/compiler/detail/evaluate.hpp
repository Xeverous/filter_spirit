#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/outcome.hpp>
#include <fs/lang/symbol_table.hpp>

#include <optional>
#include <variant>
#include <utility>

namespace fs::compiler::detail
{

[[nodiscard]] outcome<lang::color>
make_color(
	settings st,
	lang::integer r,
	lang::integer g,
	lang::integer b,
	std::optional<lang::integer> a = std::nullopt);

[[nodiscard]] outcome<lang::socket_spec>
make_socket_spec(
	settings st,
	const std::string& raw,
	lang::position_tag origin);

[[nodiscard]] inline outcome<lang::minimap_icon>
make_minimap_icon(
	lang::integer size,
	lang::suit suit,
	lang::shape shape)
{
	if (size.value != 0 && size.value != 1 && size.value != 2)
		return error(errors::invalid_integer_value{0, 2, size.value, size.origin});

	return lang::minimap_icon{size, suit, shape};
}

[[nodiscard]] outcome<lang::builtin_alert_sound>
make_builtin_alert_sound(
	settings st,
	bool positional,
	lang::integer sound_id,
	std::optional<lang::integer> volume = std::nullopt);

[[nodiscard]] outcome<lang::socket_spec>
evaluate_socket_spec_literal(
	settings st,
	boost::optional<parser::ast::common::integer_literal> int_lit,
	const parser::ast::common::identifier& iden);

[[nodiscard]] outcome<lang::object>
evaluate_sequence(
	settings st,
	const parser::ast::sf::sequence& sequence,
	const lang::symbol_table& symbols,
	int min_allowed_elements = 1,
	std::optional<int> max_allowed_elements = std::nullopt);

[[nodiscard]] outcome<lang::object>
evaluate_value_expression(
	settings st,
	const parser::ast::sf::value_expression& value_expression,
	const lang::symbol_table& symbols);

[[nodiscard]] inline lang::string
evaluate(parser::ast::rf::string_literal sl)
{
	return {static_cast<std::string>(sl), parser::position_tag_of(sl)};
}

[[nodiscard]] inline lang::boolean
evaluate(parser::ast::rf::boolean_literal bl)
{
	return {bl.value, parser::position_tag_of(bl)};
}

[[nodiscard]] inline lang::integer
evaluate(parser::ast::rf::integer_literal il)
{
	return {il.value, parser::position_tag_of(il)};
}

[[nodiscard]] inline outcome<lang::color>
evaluate(
	settings st,
	parser::ast::rf::color_literal cl)
{
	std::optional<lang::integer> a;
	if (cl.a)
		a = evaluate(*cl.a);

	return make_color(st, evaluate(cl.r), evaluate(cl.g), evaluate(cl.b), a);
}

[[nodiscard]] inline lang::rarity
evaluate(parser::ast::rf::rarity_literal rl)
{
	return {rl.value, parser::position_tag_of(rl)};
}

[[nodiscard]] inline lang::suit
evaluate(parser::ast::rf::suit_literal sl)
{
	return {sl.value, parser::position_tag_of(sl)};
}

[[nodiscard]] inline lang::shape
evaluate(parser::ast::rf::shape_literal sl)
{
	return {sl.value, parser::position_tag_of(sl)};
}

[[nodiscard]] inline lang::influence
evaluate(parser::ast::rf::influence_literal il)
{
	return {il.value, parser::position_tag_of(il)};
}

[[nodiscard]] inline outcome<lang::minimap_icon>
evaluate(parser::ast::rf::icon_literal il)
{
	return make_minimap_icon(
		detail::evaluate(il.size),
		detail::evaluate(il.suit),
		detail::evaluate(il.shape)
	);
}

template <typename T>
[[nodiscard]] outcome<T>
get_as(const lang::single_object& sobj)
{
	if (!std::holds_alternative<T>(sobj.value)) {
		return error(errors::type_mismatch{lang::object_type_of<T>(), sobj.type(), sobj.origin});
	}

	return std::get<T>(sobj.value);
}

[[nodiscard]] outcome<lang::fractional>
get_as_fractional(const lang::single_object& sobj);

[[nodiscard]] outcome<lang::socket_spec>
get_as_socket_spec(const lang::single_object& sobj);

} // namespace fs::compiler::detail
