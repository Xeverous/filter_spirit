#include <fs/generator/common.hpp>
#include <fs/generator/generator.hpp>
#include <fs/generator/make_item_filter.hpp>
#include <fs/parser/parser.hpp>
#include <fs/parser/ast_adapted.hpp> // required adaptation info for fs::log::structure_printer
#include <fs/compiler/compiler.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/symbol_table.hpp>
#include <fs/log/logger.hpp>
#include <fs/log/structure_printer.hpp>
#include <fs/utility/monadic.hpp>

#include <utility>

namespace {

using namespace fs;
using namespace fs::generator;

std::optional<fs::lang::spirit_item_filter> parse_and_compile_spirit_filter(
	std::string_view input,
	settings st,
	log::logger& logger)
{
	logger.info() << "Parsing filter template...\n";
	std::variant<parser::parsed_spirit_filter, parser::parse_failure_data> parse_result = parser::parse_spirit_filter(input);

	if (std::holds_alternative<parser::parse_failure_data>(parse_result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(parse_result), logger);
		return std::nullopt;
	}

	logger.info() << "Parse successful.\n";
	const auto& parse_data = std::get<parser::parsed_spirit_filter>(parse_result);

	if (st.print_ast)
		fs::log::structure_printer()(parse_data.ast);

	logger.info() << "Resolving filter template symbols...\n";

	compiler::diagnostics_container diagnostics_symbols;
	const boost::optional<compiler::symbol_table> symbols =
		compiler::resolve_spirit_filter_symbols(st.compile_settings, parse_data.ast.definitions, diagnostics_symbols);
	compiler::output_diagnostics(diagnostics_symbols, parse_data.metadata, logger);

	if (!symbols)
		return std::nullopt;

	logger.info() << "Symbols resolved.\n";
	logger.info() << "Compiling filter template...\n";

	compiler::diagnostics_container diagnostics_spirit_filter;
	boost::optional<lang::spirit_item_filter> spirit_filter =
		compiler::compile_spirit_filter_statements(st.compile_settings, parse_data.ast.statements, *symbols, diagnostics_spirit_filter);
	compiler::output_diagnostics(diagnostics_spirit_filter, parse_data.metadata, logger);

	return utility::to_std_optional(std::move(spirit_filter));
}

} // namespace

namespace fs::generator
{

std::optional<std::string> parse_compile_generate_spirit_filter(
	std::string_view input,
	const lang::market::item_price_report& report,
	settings st,
	log::logger& logger)
{
	std::optional<std::string> maybe_filter = parse_compile_generate_spirit_filter_no_preamble(input, report.data, st, logger);

	if (!maybe_filter)
		return std::nullopt;

	std::string& filter = *maybe_filter;
	prepend_metadata(report.metadata, filter);
	return filter;
}

std::optional<std::string> parse_compile_generate_spirit_filter_no_preamble(
	std::string_view input,
	const lang::market::item_price_data& item_price_data,
	settings st,
	log::logger& logger)
{
	logger.info() << "" << item_price_data; // add << "" to workaround calling <<(rvalue, item_price_data)

	std::optional<fs::lang::spirit_item_filter> spirit_filter = parse_and_compile_spirit_filter(input, st, logger);

	if (!spirit_filter)
		return std::nullopt;

	lang::item_filter filter = make_item_filter(*spirit_filter, item_price_data);
	logger.info() << "Compilation successful.\n";

	return item_filter_to_string(filter);
}

}
