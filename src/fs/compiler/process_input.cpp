#include "fs/parser/parser.hpp"
#include "fs/parser/ast_adapted.hpp" // required adaptation info for fs::log::structure_printer
#include "fs/compiler/process_input.hpp"
#include "fs/compiler/compiler.hpp"
#include "fs/compiler/print_error.hpp"
#include "fs/compiler/detail/filter_builder.hpp"
#include "fs/log/logger.hpp"
#include "fs/log/structure_printer.hpp"

namespace fs::compiler
{

std::optional<std::string> process_input(
	std::string_view input,
	const itemdata::item_price_data& item_price_data,
	bool print_ast,
	log::logger& logger)
{
	item_price_data.log_info(logger);
	logger.info() << "parsing filter template";
	std::optional<parser::parse_data> parse_result = parser::parse(input, logger);

	if (!parse_result)
		return std::nullopt;

	logger.info() << "parse successful";

	parser::parse_data& parse_data = *parse_result;

	if (print_ast)
		fs::log::structure_printer()(parse_data.ast);

	logger.info() << "compiling filter template";

	std::variant<lang::constants_map, error::error_variant> map_or_error = resolve_constants(parse_data.ast.constant_definitions, item_price_data);
	if (std::holds_alternative<error::error_variant>(map_or_error))
	{
		print_error(std::get<error::error_variant>(map_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	const auto& map = std::get<lang::constants_map>(map_or_error);
	const std::variant<std::vector<lang::filter_block>, error::error_variant> filter_or_error =
		detail::filter_builder::build_filter(parse_data.ast.statements, map, item_price_data);

	if (std::holds_alternative<error::error_variant>(filter_or_error))
	{
		print_error(std::get<error::error_variant>(filter_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	logger.info() << "compilation successful";

	const auto& blocks = std::get<std::vector<lang::filter_block>>(filter_or_error);
	return generate_filter(blocks);
}

}
