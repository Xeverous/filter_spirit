#pragma once

#include "fs/parser/ast.hpp"
#include "fs/log/buffered_logger.hpp"

#include <string>

namespace fsut::parser
{

const std::string& minimal_input();

class parser_fixture
{
protected:
	fs::parser::ast::ast_type parse(const std::string& input);

	fs::buffered_logger log_buf;
};

}
