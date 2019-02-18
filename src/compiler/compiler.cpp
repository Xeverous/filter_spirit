#include "compiler/compiler.hpp"
#include "compiler/rules.hpp"
#include "compiler/generic.hpp"
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "parser/parser.hpp"
#include "lang/constants_map.hpp"
#include "print/compile_error.hpp"
#include "print/structure_printer.hpp"
#include "utility/holds_alternative.hpp"
#include <cassert>
#include <string_view>
#include <utility>

namespace
{

using namespace fs;

[[nodiscard]]
std::variant<lang::constants_map, compiler::error::error_variant> resolve_constants(
	const std::vector<parser::ast::constant_definition>& constant_definitions,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
std::optional<compiler::error::error_variant> add_constant_from_definition(
	const parser::ast::constant_definition& def,
	const parser::lookup_data& lookup_data,
	lang::constants_map& map);

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
std::optional<compiler::error::error_variant> add_constant_from_definition(
	const past::constant_definition& /* def */,
	const parser::lookup_data& /* lookup_data */,
	lang::constants_map& /* map */)
{
//	const past::identifier& wanted_name = def.name;
//	const past::value_expression& value_expression = def.value;
//
//	// const past::type_expression& wanted_type = def.type; // FIXME syntax redesign
//
//	const auto wanted_name_it = map.find(wanted_name.value); // C++17: use if (expr; cond)
//	if (wanted_name_it != map.end())
//	{
//		assert(wanted_name_it->second.name_origin);
//		const parser::range_type place_of_original_name = *wanted_name_it->second.name_origin;
//		const parser::range_type place_of_duplicated_name = lookup_data.position_of(wanted_name);
//		return error::name_already_exists{place_of_duplicated_name, place_of_original_name};
//	}
//
//	std::variant<lang::object, error::error_variant> expr_result =
//		expression_to_object(value_expression, lookup_data, map);
//
//	if (std::holds_alternative<error::error_variant>(expr_result))
//		return std::get<error::error_variant>(expr_result);
//
//	lang::object_type lang_type = type_expression_to_type(wanted_type);
//	lang::object& object = std::get<lang::object>(expr_result);
//
//	std::variant<lang::object, error::error_variant> construct_result =
//		construct_object_of_type(lang_type, std::move(object));
//
//	if (std::holds_alternative<error::error_variant>(construct_result))
//		return std::get<error::error_variant>(construct_result);
//
//	lang::object& final_object = std::get<lang::object>(construct_result);
//	final_object.type_origin  = lookup_data.position_of(wanted_type);
//	final_object.value_origin = lookup_data.position_of(value_expression);
//	final_object.name_origin  = lookup_data.position_of(wanted_name);
//
//	const auto pair = map.emplace(wanted_name.value, std::move(final_object));
//	assert(pair.second);
	assert(false);
	return std::nullopt; // FIXME implement
}

std::variant<lang::constants_map, compiler::error::error_variant> resolve_constants(
	const std::vector<parser::ast::constant_definition>& constant_definitions,
	const parser::lookup_data& lookup_data)
{
	lang::constants_map map;

	for (const past::constant_definition& def : constant_definitions)
	{
		const std::optional<compiler::error::error_variant> error =
			add_constant_from_definition(def, lookup_data, map);

		if (error)
			return *error;
	}

	return map;
}

[[nodiscard]]
bool semantic_analysis(const parser::parse_data& /* parse_data */)
{
	return false; // FIXME implement
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
	if (true) // allow easy switching on/off for now (before full implemenation of command line args)
		print::structure_printer()(parse_data.ast);

	std::variant<lang::constants_map, error::error_variant> map_or_error = resolve_constants(parse_data.ast.constant_definitions, parse_data.lookup_data);

	if (std::holds_alternative<error::error_variant>(map_or_error))
	{
		print::compile_error(std::get<error::error_variant>(map_or_error), parse_data.lookup_data.get_range_of_whole_content(), error_stream);
		return false;
	}

	if (!semantic_analysis(parse_data))
		return false;

	return true;
}

}
