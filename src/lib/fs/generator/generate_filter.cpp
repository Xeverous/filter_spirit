#include <fs/generator/generate_filter.hpp>
#include <fs/generator/generator.hpp>
#include <fs/generator/make_filter.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/parser/ast_adapted.hpp> // required adaptation info for fs::log::structure_printer
#include <fs/compiler/compiler.hpp>
#include <fs/compiler/outcome.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/log/logger.hpp>
#include <fs/log/structure_printer.hpp>

namespace fs::generator::sf
{

std::optional<fs::lang::spirit_item_filter> parse_spirit_filter(
	std::string_view input,
	settings st,
	log::logger& logger)
{
	logger.info() << "parsing filter template\n";
	std::variant<parser::sf::parse_success_data, parser::parse_failure_data> parse_result = parser::sf::parse(input);

	if (std::holds_alternative<parser::parse_failure_data>(parse_result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(parse_result), logger);
		return std::nullopt;
	}

	logger.info() << "parse successful\n";
	const auto& parse_data = std::get<parser::sf::parse_success_data>(parse_result);

	if (st.print_ast)
		fs::log::structure_printer()(parse_data.ast);

	logger.info() << "resolving filter template symbols\n";

	const compiler::outcome<lang::symbol_table> symbols_outcome =
		compiler::resolve_spirit_filter_symbols(st.compile_settings, parse_data.ast.definitions);
	compiler::output_logs(symbols_outcome.logs(), parse_data.lookup, logger);

	if (!symbols_outcome.has_result())
		return std::nullopt;

	logger.info() << "compiling filter template\n";

	compiler::outcome<lang::spirit_item_filter> spirit_filter_outcome =
		compiler::compile_spirit_filter_statements(st.compile_settings, parse_data.ast.statements, symbols_outcome.result());
	compiler::output_logs(spirit_filter_outcome.logs(), parse_data.lookup, logger);

	if (!spirit_filter_outcome.has_result())
		return std::nullopt;

	return std::move(spirit_filter_outcome).result();
}

std::optional<std::string> generate_filter(
	std::string_view input,
	const lang::market::item_price_report& report,
	settings st,
	log::logger& logger)
{
	std::optional<std::string> maybe_filter = generate_filter_without_preamble(input, report.data, st, logger);

	if (!maybe_filter)
		return std::nullopt;

	std::string& filter = *maybe_filter;
	prepend_metadata(report.metadata, filter);
	return filter;
}

std::optional<std::string> generate_filter_without_preamble(
	std::string_view input,
	const lang::market::item_price_data& item_price_data,
	settings st,
	log::logger& logger)
{
	logger.info() << "" << item_price_data; // add << "" to workaround calling <<(rvalue, item_price_data)

	std::optional<fs::lang::spirit_item_filter> spirit_filter = parse_spirit_filter(input, st, logger);

	if (!spirit_filter)
		return std::nullopt;

	lang::item_filter filter = make_filter(*spirit_filter, item_price_data);
	logger.info() << "compilation successful\n";

	return to_string(filter);
}

}
