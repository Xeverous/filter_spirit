#include <fs/generator/generate_filter.hpp>
#include <fs/generator/generator.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/parser/ast_adapted.hpp> // required adaptation info for fs::log::structure_printer
#include <fs/compiler/error.hpp>
#include <fs/compiler/print_error.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/log/logger.hpp>
#include <fs/log/structure_printer.hpp>

namespace fs::generator::sf
{

std::optional<std::string> generate_filter(
	std::string_view input,
	const lang::item_price_info& item_price_info,
	options options,
	log::logger& logger)
{
	std::optional<std::string> maybe_filter = generate_filter_without_preamble(input, item_price_info.data, options, logger);

	if (!maybe_filter)
		return std::nullopt;

	std::string& filter = *maybe_filter;
	prepend_metadata(item_price_info.metadata, filter);
	return filter;
}

std::optional<std::string> generate_filter_without_preamble(
	std::string_view input,
	const lang::item_price_data& item_price_data,
	options options,
	log::logger& logger)
{
	logger.info() << "" << item_price_data; // add << "" to workaround calling <<(rvalue, item_price_data)
	logger.info() << "parsing filter template\n";
	std::variant<parser::sf::parse_success_data, parser::parse_failure_data> parse_result = parser::sf::parse(input);

	if (std::holds_alternative<parser::parse_failure_data>(parse_result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(parse_result), logger);
		return std::nullopt;
	}

	logger.info() << "parse successful\n";
	const auto& parse_data = std::get<parser::sf::parse_success_data>(parse_result);

	if (options.print_ast)
		fs::log::structure_printer()(parse_data.ast);

	logger.info() << "compiling filter template\n";

	std::variant<lang::symbol_table, compiler::compile_error> symbols_or_error =
		compiler::resolve_spirit_filter_symbols(parse_data.ast.definitions);
	if (std::holds_alternative<compiler::compile_error>(symbols_or_error)) {
		compiler::print_error(std::get<compiler::compile_error>(symbols_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	const auto& map = std::get<lang::symbol_table>(symbols_or_error);
	const std::variant<lang::item_filter, compiler::compile_error> filter_or_error =
		compiler::compile_spirit_filter(parse_data.ast.statements, map);

	if (std::holds_alternative<compiler::compile_error>(filter_or_error)) {
		compiler::print_error(std::get<compiler::compile_error>(filter_or_error), parse_data.lookup_data, logger);
		return std::nullopt;
	}

	logger.info() << "compilation successful\n";

	const auto& filter = std::get<lang::item_filter>(filter_or_error);
	return to_raw_filter(filter);
}

}
