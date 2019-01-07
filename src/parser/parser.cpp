#include "ast_adapted.hpp"
#include "parser.hpp"
#include "grammar.hpp"
#include "ast_printer.hpp"
#include <iostream>
#include <functional>

namespace fs::parser
{

ast::ast_type parse(const std::string& source, position_cache& positions, bool& result)
{
	iterator_type it = source.begin();
	const iterator_type end = source.end();
	fs::ast::ast_type ast;

	const auto parser = x3::with<position_cache_tag>(std::ref(positions))[fs::grammar()];

	result = x3::phrase_parse(
		it,
		end,
		parser,
		fs::skipper(),
		ast
	);

	if (it != end or !result) // fail if we did not get a full match
	{
		std::cout << "parse failure at pos: " << it - source.begin() << "\n";
		result = false;
	}

	return ast;
}

void print_ast(const ast::ast_type& ast)
{
	ast::printer()(ast);
}

}
