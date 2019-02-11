#include "compiler/compiler.hpp"
#include "compiler/process_input.hpp"
#include "compiler/rules.hpp"
#include "compiler/generic.hpp"
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "parser/parser.hpp"
#include "print/compile_error.hpp"
#include "utility/holds_alternative.hpp"
#include <cassert>
#include <string_view>
#include <utility>

namespace fs::compiler
{

namespace past = parser::ast;

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
std::optional<error::error_variant> add_constant_from_definition(
	const past::constant_definition& def,
	const parser::lookup_data& lookup_data,
	constants_map& map)
{
	const past::identifier& wanted_name = def.name;
	const past::type_expression& wanted_type = def.type;
	const past::value_expression& value_expression = def.value;

	const auto wanted_name_it = map.find(wanted_name.value); // C++17: use if (expr; cond)
	if (wanted_name_it != map.end())
	{
		assert(wanted_name_it->second.name_origin);
		const parser::range_type place_of_original_name = *wanted_name_it->second.name_origin;
		const parser::range_type place_of_duplicated_name = lookup_data.position_of(wanted_name);
		return error::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	std::variant<lang::object, error::error_variant> expr_result =
		expression_to_object(value_expression, lookup_data, map);

	if (std::holds_alternative<error::error_variant>(expr_result))
		return std::get<error::error_variant>(expr_result);

	lang::object_type lang_type = type_expression_to_type(wanted_type);
	lang::object& object = std::get<lang::object>(expr_result);

	std::variant<lang::object, error::error_variant> construct_result =
		construct_object_of_type(lang_type, std::move(object));

	if (std::holds_alternative<error::error_variant>(construct_result))
		return std::get<error::error_variant>(construct_result);

	lang::object& final_object = std::get<lang::object>(construct_result);
	final_object.type_origin  = lookup_data.position_of(wanted_type);
	final_object.value_origin = lookup_data.position_of(value_expression);
	final_object.name_origin  = lookup_data.position_of(wanted_name);

	const auto pair = map.emplace(wanted_name.value, std::move(final_object));
	assert(pair.second);
	return std::nullopt;
}

std::optional<constants_map> parse_constants(
	const past::constant_definition_list& constants_list,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	constants_map map;

	for (const past::constant_definition& line : constants_list.constant_definitions)
	{
		const std::optional<error::error_variant> error =
			add_constant_from_definition(line, lookup_data, map);

		if (error)
		{
			print::compile_error(*error, lookup_data.get_range_of_whole_content(), error_stream);
			return std::nullopt;
		}
	}

	return map;
}

bool semantic_analysis(
	const past::ast_type& ast,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	std::optional<constants_map> map = parse_constants(ast.constants_list, lookup_data, error_stream);

	if (!map)
		return false;

	for (const auto& pair : *map)
	{
		error_stream << pair.first << "\n";
	}

	std::variant<std::vector<lang::filter_block>, error::error_variant> filter_content =
		compile_rules(ast.actions, ast.blocks, *map, lookup_data);

	return !std::holds_alternative<error::error_variant>(filter_content);
}

}
