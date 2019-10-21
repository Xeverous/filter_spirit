#include <fs/generator/generate_filter.hpp>
#include <fs/generator/generator.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/parser/ast_adapted.hpp> // required adaptation info for fs::log::structure_printer
#include <fs/compiler/error.hpp>
#include <fs/compiler/print_error.hpp>
#include <fs/compiler/resolve_symbols.hpp>
#include <fs/compiler/build_filter_blocks.hpp>
#include <fs/log/logger.hpp>
#include <fs/log/structure_printer.hpp>

namespace fs::generator
{

std::optional<std::string> generate_filter(
	std::string_view input,
	const lang::item_price_data& item_price_data,
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
	const lang::item_price_data& item_price_data,
	options options,
	log::logger& logger)
{
	item_price_data.log_info(logger);
	logger.info() << "parsing filter template";
	std::variant<parser::parse_success_data, parser::parse_failure_data> parse_result = parser::parse(input);

	if (std::holds_alternative<parser::parse_failure_data>(parse_result))
	{
		parser::print_parse_errors(std::get<parser::parse_failure_data>(parse_result), logger);
		return std::nullopt;
	}

	logger.info() << "parse successful";
	const auto& parse_data = std::get<parser::parse_success_data>(parse_result);

	if (options.print_ast)
		fs::log::structure_printer()(parse_data.ast);

	logger.info() << "compiling filter template";

	std::variant<lang::symbol_table, compiler::compile_error> symbols_or_error =
		compiler::resolve_symbols(parse_data.ast.definitions, item_price_data);
	if (std::holds_alternative<compiler::compile_error>(symbols_or_error))
	{
		compiler::print_error(std::get<compiler::compile_error>(symbols_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	const auto& map = std::get<lang::symbol_table>(symbols_or_error);
	const std::variant<std::vector<lang::filter_block>, compiler::compile_error> filter_or_error =
		compiler::build_filter_blocks(parse_data.ast.statements, map, item_price_data);

	if (std::holds_alternative<compiler::compile_error>(filter_or_error))
	{
		compiler::print_error(std::get<compiler::compile_error>(filter_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	logger.info() << "compilation successful";

	const auto& blocks = std::get<std::vector<lang::filter_block>>(filter_or_error);
	return assemble_blocks_to_raw_filter(blocks);
}

}
