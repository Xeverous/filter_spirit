#pragma once

#include <fs/lang/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/error.hpp>
#include <fs/compiler/compiler.hpp>

#include <string>
#include <string_view>
#include <variant>

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
	std::variant<lang::symbol_table, compiler::compile_error>
	resolve_symbols(const std::vector<parser::ast::sf::definition>& defs)
	{
		return compiler::resolve_spirit_filter_symbols(defs);
	}
};

}
