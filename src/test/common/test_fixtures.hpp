#pragma once

#include <fs/parser/parser.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/symbol_table.hpp>

#include <string>
#include <string_view>

namespace fs::test
{

const std::string& minimal_input();

class parser_fixture
{
protected:
	static
	parser::parsed_spirit_filter parse(std::string_view input);
};

class compiler_fixture : public parser_fixture
{
protected:
	static
	std::optional<compiler::symbol_table>
	resolve_symbols(
		compiler::settings st,
		const std::vector<parser::ast::sf::definition>& defs,
		compiler::diagnostics_store& diagnostics)
	{
		return compiler::resolve_spirit_filter_symbols(st, defs, diagnostics);
	}

	static
	std::optional<compiler::symbol_table>
	resolve_symbols(
		const std::vector<parser::ast::sf::definition>& defs,
		compiler::diagnostics_store& diagnostics)
	{
		return resolve_symbols(compiler::settings{}, defs, diagnostics);
	}
};

}
