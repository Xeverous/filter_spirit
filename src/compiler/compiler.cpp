#include "compiler/compiler.hpp"
#include "compiler/generic.hpp"
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "parser/parser.hpp"
#include "print/compile_error.hpp"
#include "print/structure_printer.hpp"
#include "utility/holds_alternative.hpp"
#include <cassert>
#include <string_view>
#include <utility>

namespace
{

namespace past = fs::parser::ast;

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
std::optional<fs::compiler::error::error_variant> add_constant_from_definition(
	const fs::parser::ast::constant_definition& def,
	const fs::parser::lookup_data& lookup_data,
	fs::compiler::constants_map& map)
{
	const past::identifier& wanted_name = def.name;
	const past::type_expression& wanted_type = def.type;
	const past::value_expression& value_expression = def.value;

	const auto wanted_name_it = map.find(wanted_name.value); // C++17: use if (expr; cond)
	if (wanted_name_it != map.end())
	{
		const fs::parser::range_type place_of_duplicated_name = lookup_data.position_of(def.name);
		assert(wanted_name_it->second.name_origin);
		const fs::parser::range_type place_of_original_name = *wanted_name_it->second.name_origin;
		return fs::compiler::error::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	std::variant<fs::lang::object, fs::compiler::error::error_variant> expr_result =
		fs::compiler::expression_to_object(value_expression, lookup_data, map);

	if (std::holds_alternative<fs::compiler::error::error_variant>(expr_result))
		return std::get<fs::compiler::error::error_variant>(expr_result);

	fs::lang::object_type lang_type = fs::compiler::type_expression_to_type(wanted_type);
	fs::lang::object& object = std::get<fs::lang::object>(expr_result);

	std::variant<fs::lang::object, fs::compiler::error::error_variant> construct_result =
		fs::compiler::construct_object_of_type(lang_type, std::move(object));

	if (std::holds_alternative<fs::compiler::error::error_variant>(construct_result))
		return std::get<fs::compiler::error::error_variant>(construct_result);

	fs::lang::object& final_object = std::get<fs::lang::object>(construct_result);
	final_object.type_origin  = lookup_data.position_of(wanted_type);
	final_object.value_origin = lookup_data.position_of(value_expression);
	final_object.name_origin  = lookup_data.position_of(wanted_name);

	const auto pair = map.emplace(wanted_name.value, std::move(final_object));
	assert(pair.second);
	return std::nullopt;
}

} // namespace

namespace fs::compiler
{

[[nodiscard]]
std::optional<compiler::constants_map> parse_constants(
	const parser::ast::ast_type& ast,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	compiler::constants_map map;

	for (const parser::ast::code_line& line : ast)
	{
		if (line.value)
		{
			const std::optional<fs::compiler::error::error_variant> error =
				add_constant_from_definition(*line.value, lookup_data, map);

			if (error)
			{
				print::compile_error(*error, lookup_data.get_range_of_whole_content(), error_stream);
				return std::nullopt;
			}
		}
	}

	return map;
}

bool semantic_analysis(const parser::ast::ast_type& ast, const parser::lookup_data& lookup_data, std::ostream& error_stream)
{
	std::optional<compiler::constants_map> map = parse_constants(ast, lookup_data, error_stream);

	if (!map)
		return false;

	for (const auto& pair : *map)
	{
		error_stream << pair.first << "\n";
	}

	return true;
}

bool compile(const std::string& file_content, std::ostream& error_stream)
{
	std::optional<std::pair<parser::ast::ast_type, parser::lookup_data>> parse_result = parser::parse(file_content, error_stream);

	if (!parse_result)
		return false;

	auto& ast = (*parse_result).first;
	auto& lookup_data = (*parse_result).second;

	if (false) // don't print now, but keep to test that code compiles
		print::structure_printer()(ast);

	if (!semantic_analysis(ast, lookup_data, error_stream))
		return false;

	return true;
}

}
