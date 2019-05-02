#include "fs/generator/generate_filter.hpp"
#include "fs/generator/generator.hpp"
#include "fs/parser/parser.hpp"
#include "fs/parser/ast_adapted.hpp" // required adaptation info for fs::log::structure_printer
#include "fs/compiler/compiler.hpp"
#include "fs/compiler/print_error.hpp"
#include "fs/log/logger.hpp"
#include "fs/log/structure_printer.hpp"

namespace fs::generator
{

std::optional<std::string> generate_filter(
	std::string_view input,
	const itemdata::item_price_data& item_price_data,
	options options,
	metadata metadata,
	log::logger& logger)
{
	std::optional<std::string> maybe_filter = generate_filter_without_preamble(input, item_price_data, options, logger);

	if (!maybe_filter)
		return std::nullopt;

	std::string& filter = *maybe_filter;
	prepend_metadata(metadata, filter);
	return filter;
}

std::optional<std::string> generate_filter_without_preamble(
	std::string_view input,
	const itemdata::item_price_data& item_price_data,
	options options,
	log::logger& logger)
{
	item_price_data.log_info(logger);
	logger.info() << "parsing filter template";
	std::optional<parser::parse_data> parse_result = parser::parse(input, logger);

	if (!parse_result)
		return std::nullopt;

	logger.info() << "parse successful";

	parser::parse_data& parse_data = *parse_result;

	if (options.print_ast)
		fs::log::structure_printer()(parse_data.ast);

	logger.info() << "compiling filter template";

	std::variant<lang::constants_map, compiler::error::error_variant> map_or_error =
		compiler::resolve_constants(parse_data.ast.constant_definitions, item_price_data);
	if (std::holds_alternative<compiler::error::error_variant>(map_or_error))
	{
		compiler::print_error(std::get<compiler::error::error_variant>(map_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	const auto& map = std::get<lang::constants_map>(map_or_error);
	const std::variant<std::vector<lang::filter_block>, compiler::error::error_variant> filter_or_error =
		compiler::compile_statements(parse_data.ast.statements, map, item_price_data);

	if (std::holds_alternative<compiler::error::error_variant>(filter_or_error))
	{
		compiler::print_error(std::get<compiler::error::error_variant>(filter_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	logger.info() << "compilation successful";

	const auto& blocks = std::get<std::vector<lang::filter_block>>(filter_or_error);
	return assemble_blocks_to_raw_filter(blocks);
}

}
