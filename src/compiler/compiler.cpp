#include "compiler/compiler.hpp"
#include "compiler/evaluate.hpp"
#include "compiler/error.hpp"
#include "compiler/filter_builder.hpp"
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "parser/parser.hpp"
#include "lang/constants_map.hpp"
#include "print/compile_error.hpp"
#include "utility/holds_alternative.hpp"
#include <cassert>
#include <string_view>
#include <utility>

namespace
{

using namespace fs;

namespace ast = parser::ast;

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
[[nodiscard]]
std::optional<compiler::error::error_variant> add_constant_from_definition(
	const ast::constant_definition& def,
	lang::constants_map& map)
{
	const ast::identifier& wanted_name = def.name;
	const ast::value_expression& value_expression = def.value;

	const auto it = map.find(wanted_name.value); // C++17: use if (expr; cond)
	if (it != map.end())
	{
		assert(it->second.name_origin);
		lang::position_tag place_of_original_name = parser::get_position_info(*it->second.name_origin);
		lang::position_tag place_of_duplicated_name = parser::get_position_info(wanted_name);
		return compiler::error::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	std::variant<lang::object, compiler::error::error_variant> expr_result =
		compiler::evaluate_value_expression(value_expression, map);

	if (std::holds_alternative<compiler::error::error_variant>(expr_result))
		return std::get<compiler::error::error_variant>(expr_result);

	lang::object& object = std::get<lang::object>(expr_result);
	object.name_origin = parser::get_position_info(wanted_name);
	const auto pair = map.emplace(wanted_name.value, std::move(object));
	assert(pair.second);
	return std::nullopt;
}

[[nodiscard]]
std::variant<lang::constants_map, compiler::error::error_variant> resolve_constants(
	const std::vector<parser::ast::constant_definition>& constant_definitions)
{
	lang::constants_map map;

	for (const ast::constant_definition& def : constant_definitions)
	{
		const std::optional<compiler::error::error_variant> error =
			add_constant_from_definition(def, map);

		if (error)
			return *error;
	}

	return map;
}

} // namespace

namespace fs::compiler
{

bool process_input(const std::string& file_content, std::ostream& error_stream)
{
	std::optional<parser::parse_data> parse_result = parser::parse(file_content, error_stream);

	if (!parse_result)
		return false;

	parser::parse_data& parse_data = *parse_result;
	std::variant<lang::constants_map, error::error_variant> map_or_error = resolve_constants(parse_data.ast.constant_definitions);

	if (std::holds_alternative<error::error_variant>(map_or_error))
	{
		print::compile_error(std::get<error::error_variant>(map_or_error), parse_data.lookup_data, error_stream);
		return false;
	}

	auto& map = std::get<lang::constants_map>(map_or_error);
	filter_builder fb(std::move(parse_data.ast.statements), std::move(map));
	std::optional<error::error_variant> build_error = fb.build_filter();
	if (build_error)
	{
		print::compile_error(*build_error, parse_data.lookup_data, error_stream);
		return false;
	}

	return true;
}

}
