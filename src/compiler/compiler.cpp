#include "compiler.hpp"
#include "parser/parser.hpp"
#include <unordered_map>

namespace fs
{

bool compiler::compile()
{
	bool result;
	ast::ast_type ast = parser::parse(file_content, result);

	if (!result)
		return false;

	if (!semantic_analysis(ast))
		return false;

	return true;
}

bool compiler::semantic_analysis(const ast::ast_type& ast)
{

}

}
