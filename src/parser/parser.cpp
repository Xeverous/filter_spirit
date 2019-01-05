#include "parser.hpp"
#include "grammar.hpp"
#include <iostream>
#include <functional>

namespace fs::parser
{

fs::ast::ast_type parse(const std::string& source, position_cache& positions)
{
	iterator_type it = source.begin();
	const iterator_type end = source.end();
	fs::ast::ast_type ast;

	const auto parser = x3::with<position_cache_tag>(std::ref(positions))[fs::grammar()];

	const bool result = x3::phrase_parse(
		it,
		end,
		parser,
		fs::skipper(),
		ast
	);

	if (it != end or !result) // fail if we did not get a full match
	{
		std::cout << "parse failure at pos: " << it - source.begin() << "\n";
	}

	return ast;
}

}
