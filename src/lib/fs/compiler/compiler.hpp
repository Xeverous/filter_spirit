#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/log/logger.hpp>
#include <fs/parser/ast.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>

#include <optional>
#include <vector>

namespace fs::compiler
{

// parsed_real_filter => real_filter_representation
[[nodiscard]] std::optional<lang::item_filter>
compile_real_filter(
	settings st,
	const parser::ast::rf::ast_type& ast,
	diagnostics_store& diagnostics);

// parsed_spirit_filter.definitions => symbol_table
// (symbol_table can be edited before proceeding)
[[nodiscard]] std::optional<symbol_table>
resolve_spirit_filter_symbols(
	settings st,
	const std::vector<parser::ast::sf::definition>& definitions,
	diagnostics_store& diagnostics);

// parsed_spirit_filter.statements + symbol_table => spirit_filter_representation
// (representation can be reused with different item_price_data)
[[nodiscard]] std::optional<lang::spirit_item_filter>
compile_spirit_filter_statements(
	settings st,
	const std::vector<parser::ast::sf::statement>& statements,
	const symbol_table& symbols,
	diagnostics_store& diagnostics);

// spirit_filter_representation + item_price_data => real_filter_representation
[[nodiscard]] lang::item_filter
make_item_filter(
	const lang::spirit_item_filter& filter_template,
	const lang::market::item_price_data& item_price_data);

// real_filter_representation => string
[[nodiscard]] std::string
item_filter_to_string_without_preamble(
	const lang::item_filter& filter);

// real_filter_representation => preamble + output_string
[[nodiscard]] std::string
item_filter_to_string_with_preamble(
	const lang::item_filter& filter,
	const lang::market::item_price_metadata& item_price_metadata);

// end-to-end function: input_string => output_string
// (no version/config/about preamble in generated file contents)
[[nodiscard]] std::optional<std::string>
parse_compile_generate_spirit_filter_without_preamble(
	std::string_view input,
	const lang::market::item_price_data& item_price_data,
	settings st,
	log::logger& logger);

// end-to-end function: input_string => output_string
[[nodiscard]] std::optional<std::string>
parse_compile_generate_spirit_filter_with_preamble(
	std::string_view input,
	const lang::market::item_price_report& report,
	settings st,
	log::logger& logger);

}
