#pragma once

#include "fs/log/buffered_logger.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/parser/parser.hpp"
#include "fs/compiler/error.hpp"

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

	fs::log::buffered_logger log_buf;
};

class compiler_fixture : public parser_fixture
{
protected:
	static
	std::variant<fs::lang::constants_map, fs::compiler::error::error_variant>
	resolve_constants(const std::vector<fs::parser::ast::constant_definition>& defs);
};

}
