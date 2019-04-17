#include "fs/compiler/compiler.hpp"
#include "fs/compiler/evaluate.hpp"
#include "fs/compiler/error.hpp"
#include "fs/compiler/filter_builder.hpp"
#include "fs/parser/ast.hpp"
#include "fs/parser/config.hpp"
#include "fs/parser/parser.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/print/compile_error.hpp"
#include "fs/print/structure_printer.hpp"
#include "fs/utility/holds_alternative.hpp"

#include <cassert>
#include <string_view>
#include <utility>
#include <sstream>

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
	lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	const ast::identifier& wanted_name = def.name;
	const ast::value_expression& value_expression = def.value;

	const auto it = map.find(wanted_name.value); // C++17: use if (expr; cond)
	if (it != map.end())
	{
		const lang::position_tag place_of_original_name = parser::get_position_info(it->second.name_origin);
		const lang::position_tag place_of_duplicated_name = parser::get_position_info(wanted_name);
		return compiler::error::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	std::variant<lang::object, compiler::error::error_variant> expr_result =
		compiler::evaluate_value_expression(value_expression, map, item_price_data);

	if (std::holds_alternative<compiler::error::error_variant>(expr_result))
		return std::get<compiler::error::error_variant>(expr_result);

	const auto pair = map.emplace(
		wanted_name.value,
		lang::named_object{
			std::get<lang::object>(std::move(expr_result)),
			parser::get_position_info(wanted_name)});
	assert(pair.second);
	return std::nullopt;
}

[[nodiscard]]
std::variant<lang::constants_map, compiler::error::error_variant> resolve_constants(
	const std::vector<parser::ast::constant_definition>& constant_definitions,
	const itemdata::item_price_data& item_price_data)
{
	lang::constants_map map;

	for (const ast::constant_definition& def : constant_definitions)
	{
		const std::optional<compiler::error::error_variant> error =
			add_constant_from_definition(def, map, item_price_data);

		if (error)
			return *error;
	}

	return map;
}

[[nodiscard]]
std::string generate_filter(const std::vector<lang::filter_block>& blocks)
{
	std::stringstream ss;

	for (const lang::filter_block& block : blocks)
		block.generate(ss);

	return ss.str();
}

} // namespace

namespace fs::compiler
{

std::optional<std::string> process_input(
	const std::string& input,
	const itemdata::item_price_data& item_price_data,
	bool print_ast,
	logger& logger)
{
	item_price_data.log_info(logger);
	logger.info() << "parsing filter template";
	std::optional<parser::parse_data> parse_result = parser::parse(input, logger);

	if (!parse_result)
		return std::nullopt;

	logger.info() << "parse successful";

	parser::parse_data& parse_data = *parse_result;

	if (print_ast)
		fs::print::structure_printer()(parse_data.ast);

	logger.info() << "compiling filter template";

	std::variant<lang::constants_map, error::error_variant> map_or_error = resolve_constants(parse_data.ast.constant_definitions, item_price_data);
	if (std::holds_alternative<error::error_variant>(map_or_error))
	{
		print::compile_error(std::get<error::error_variant>(map_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	const auto& map = std::get<lang::constants_map>(map_or_error);
	const std::variant<std::vector<lang::filter_block>, error::error_variant> filter_or_error =
		filter_builder::build_filter(parse_data.ast.statements, map, item_price_data);

	if (std::holds_alternative<error::error_variant>(filter_or_error))
	{
		print::compile_error(std::get<error::error_variant>(filter_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	logger.info() << "compilation successful";

	const auto& blocks = std::get<std::vector<lang::filter_block>>(filter_or_error);
	return generate_filter(blocks);
}

}
