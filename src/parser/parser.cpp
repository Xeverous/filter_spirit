#include "parser.hpp"
#include "grammar.hpp"
#include "ast_printer.hpp"
#include <iostream>

namespace fs::parser
{

ast::ast_type parse(const std::string& source, bool& result)
{
	iterator_type it = source.begin();
	const iterator_type end = source.end();
	fs::ast::ast_type ast;
	error_handler_type error_handler(it, end, std::cerr);
	const auto parser = x3::with<x3::error_handler_tag>(std::ref(error_handler))[fs::grammar()];

	result = x3::phrase_parse(
		it,
		end,
		parser,
		fs::skipper(),
		ast
	);

	if (it != end or !result) // fail if we did not get a full match
	{
		std::cout << "parse failure:\n"
			"\tstopped at character: #" << it - source.begin() << "\n"
			"\tconsumed characters :  " << it - source.begin() << "\n"
			"\tcharacters left     :  " << source.end() - it   << "\n";
		result = false;
	}

	return ast;
}

void print_ast(const ast::ast_type& ast)
{
	ast::printer()(ast);
}

}
