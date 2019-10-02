#include "fs/compiler/compiler.hpp"
#include "fs/compiler/error.hpp"
#include "fs/compiler/detail/evaluate.hpp"
#include "fs/compiler/detail/filter_builder.hpp"
#include "fs/parser/ast.hpp"
#include "fs/lang/constants_map.hpp"

#include <cassert>
#include <string_view>
#include <utility>
#include <sstream>

namespace
{

using namespace fs;
using namespace fs::compiler;
namespace ast = fs::parser::ast;

/*
 * core entry point into adding constants
 *
 * task:
 * Add object only if valid, error with appropriate information otherwise; in any case
 * after either successful addition or any error return immediately.
 *
 * flow:
 * - check that name is not already taken and error if so
 *   (it's impossible to have multiple objects with the same name)
 *   (as of now, filter's language has no scoping/name shadowing)
 * - convert expression to language object and proceed
 */
[[nodiscard]] std::optional<compile_error>
add_constant_from_definition(
	const ast::constant_definition& def,
	lang::constants_map& map,
	const lang::item_price_data& item_price_data)
{
	const ast::identifier& wanted_name = def.name;
	const ast::value_expression& value_expression = def.value;

	const auto it = map.find(wanted_name.value); // C++17: use if (expr; cond)
	if (it != map.end())
	{
		const lang::position_tag place_of_original_name = parser::get_position_info(it->second.name_origin);
		const lang::position_tag place_of_duplicated_name = parser::get_position_info(wanted_name);
		return errors::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	std::variant<lang::object, compile_error> expr_result =
		compiler::detail::evaluate_value_expression(value_expression, map, item_price_data);

	if (std::holds_alternative<compile_error>(expr_result))
		return std::get<compile_error>(std::move(expr_result));

	const auto pair = map.emplace(
		wanted_name.value,
		lang::named_object{
			std::get<lang::object>(std::move(expr_result)),
			parser::get_position_info(wanted_name)});
	assert(pair.second); // C++20: use [[assert]]
	(void) pair; // ignore insertion result in release builds
	return std::nullopt;
}

} // namespace

namespace fs::compiler
{

std::variant<lang::constants_map, compile_error>
resolve_constants(
	const std::vector<parser::ast::constant_definition>& constant_definitions,
	const lang::item_price_data& item_price_data)
{
	lang::constants_map map;

	for (const ast::constant_definition& def : constant_definitions)
	{
		std::optional<compile_error> error = add_constant_from_definition(def, map, item_price_data);

		if (error)
			return *std::move(error);
	}

	return map;
}

std::variant<std::vector<lang::filter_block>, compile_error>
compile_statements(
	const std::vector<parser::ast::statement>& statements,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data)
{
	return detail::filter_builder::build_filter(statements, map, item_price_data);
}

} // namespace fs::compiler
