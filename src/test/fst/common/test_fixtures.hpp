#pragma once

#include <fs/lang/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/error.hpp>

#include <string>
#include <string_view>
#include <variant>

namespace fst
{

const std::string& minimal_input();

class parser_fixture
{
protected:
	static
	fs::parser::parse_success_data parse(std::string_view input);
};

class compiler_fixture : public parser_fixture
{
protected:
	static
	std::variant<fs::lang::symbol_table, fs::compiler::compile_error>
	resolve_symbols(const std::vector<fs::parser::ast::definition>& defs);
};

}
