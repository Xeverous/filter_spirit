#pragma once

#include "fs/parser/ast.hpp"
#include "fs/log/buffered_logger.hpp"

#include <string>
#include <string_view>

namespace fsut::parser
{

const std::string& minimal_input();

class parser_fixture
{
protected:
	fs::parser::ast::ast_type parse(std::string_view input);

	fs::buffered_logger log_buf;
};

}
