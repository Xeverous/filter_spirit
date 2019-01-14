#pragma once
#include "parser/parser.hpp"
#include <string>
#include <utility>

namespace fs
{

class compiler
{
public:
	compiler(std::string file_content)
	: file_content(std::move(file_content))
	{
	}

	bool compile();

private:
	bool semantic_analysis(const ast::ast_type& ast);

	std::string file_content;
};

}
