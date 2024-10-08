#pragma once

#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>

#include <boost/optional.hpp>

#include <variant>
#include <utility>

namespace fs::compiler::detail
{

[[nodiscard]] boost::optional<lang::integer>
make_integer_in_range(
	lang::integer int_obj,
	int min_allowed_value,
	int max_allowed_value,
	diagnostics_store& diagnostics);

[[nodiscard]] boost::optional<lang::socket_spec>
make_socket_spec(
	settings st,
	const std::string& raw,
	lang::position_tag origin,
	diagnostics_store& diagnostics);

[[nodiscard]] inline lang::string
evaluate(const parser::ast::rf::string& str)
{
	return {str.value, parser::position_tag_of(str)};
}

[[nodiscard]] inline lang::string
evaluate(const parser::ast::common::string_literal& sl)
{
	return {static_cast<std::string>(sl), parser::position_tag_of(sl)};
}

[[nodiscard]] inline lang::boolean
evaluate(parser::ast::common::boolean_literal bl)
{
	return {bl.value, parser::position_tag_of(bl)};
}

[[nodiscard]] inline lang::integer
evaluate(parser::ast::common::integer_literal il)
{
	return {il.value, parser::position_tag_of(il)};
}

[[nodiscard]] inline lang::fractional
evaluate(parser::ast::common::floating_point_literal fpl)
{
	return {fpl.value, parser::position_tag_of(fpl)};
}

[[nodiscard]] inline lang::rarity
evaluate(parser::ast::common::rarity_literal rl)
{
	return {rl.value, parser::position_tag_of(rl)};
}

[[nodiscard]] inline lang::suit
evaluate(parser::ast::common::suit_literal sl)
{
	return {sl.value, parser::position_tag_of(sl)};
}

[[nodiscard]] inline lang::shape
evaluate(parser::ast::common::shape_literal sl)
{
	return {sl.value, parser::position_tag_of(sl)};
}

[[nodiscard]] inline lang::shaper_voice_line
evaluate(parser::ast::common::shaper_voice_line_literal svll)
{
	return {svll.value, parser::position_tag_of(svll)};
}

[[nodiscard]] inline lang::influence
evaluate(parser::ast::common::influence_literal il)
{
	return {il.value, parser::position_tag_of(il)};
}

[[nodiscard]] inline lang::temp_type
evaluate(parser::ast::common::temp_literal tl)
{
	return {parser::position_tag_of(tl)};
}

[[nodiscard]] inline lang::none_type
evaluate(parser::ast::common::none_literal nl)
{
	return {parser::position_tag_of(nl)};
}

[[nodiscard]] boost::optional<lang::socket_spec>
evaluate(
	settings st,
	const parser::ast::common::socket_spec_literal& literal,
	diagnostics_store& diagnostics);

[[nodiscard]] boost::optional<lang::single_object>
evaluate(
	settings st,
	const parser::ast::common::literal_expression& expression,
	diagnostics_store& diagnostics);

[[nodiscard]] boost::optional<lang::object>
evaluate_literal_sequence(
	settings st,
	const parser::ast::rf::literal_sequence& sequence,
	diagnostics_store& diagnostics);

[[nodiscard]] boost::optional<lang::object>
evaluate_sequence(
	settings st,
	const parser::ast::sf::sequence& sequence,
	const symbol_table& symbols,
	diagnostics_store& diagnostics);

[[nodiscard]] bool
check_object_size(
	const lang::object& obj,
	int min_allowed_elements,
	boost::optional<int> max_allowed_elements,
	diagnostics_store& diagnostics);

template <typename T>
[[nodiscard]] boost::optional<T>
get_as(
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
{
	if (!std::holds_alternative<T>(sobj.value)) {
		diagnostics.push_error_type_mismatch(lang::object_type_of<T>(), sobj.type(), sobj.origin);
		return boost::none;
	}

	return std::get<T>(sobj.value);
}

template <>
[[nodiscard]] boost::optional<lang::fractional>
get_as<lang::fractional>(
	const lang::single_object& sobj,
	diagnostics_store& diagnostics);

template <>
[[nodiscard]] boost::optional<lang::socket_spec>
get_as<lang::socket_spec>(
	const lang::single_object& sobj,
	diagnostics_store& diagnostics);

} // namespace fs::compiler::detail
