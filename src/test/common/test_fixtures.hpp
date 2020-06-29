#pragma once

#include <fs/lang/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/compiler/outcome.hpp>

#include <string>
#include <string_view>

namespace fs::test
{

const std::string& minimal_input();

class parser_fixture
{
protected:
	static
	fs::parser::sf::parse_success_data parse(std::string_view input);
};

class compiler_fixture : public parser_fixture
{
protected:
	static
	compiler::outcome<lang::symbol_table>
	resolve_symbols(compiler::settings st, const std::vector<parser::ast::sf::definition>& defs)
	{
		return compiler::resolve_spirit_filter_symbols(st, defs);
	}

	static
	compiler::outcome<lang::symbol_table>
	resolve_symbols(const std::vector<parser::ast::sf::definition>& defs)
	{
		return resolve_symbols(compiler::settings{}, defs);
	}
};

}
