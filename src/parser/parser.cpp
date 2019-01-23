#include "parser/parser.hpp"
#include "parser/grammar.hpp"
#include <iostream>

namespace fs::parser
{

state_handler parse(std::string file_content, bool& result)
{
	      iterator_type it   {file_content.begin()};
	const iterator_type begin{file_content.begin()};
	const iterator_type end  {file_content.end()};
	state_handler state("", std::move(file_content));
	// note: x3::with<> must match with grammar's context_type, otherwise you will get linker errors
	const auto parser = x3::with<x3::error_handler_tag>(std::ref(state))[fs::grammar()];

	result = x3::phrase_parse(
		it,
		end,
		parser,
		fs::skipper(),
		state.get_ast()
	);

	if (it != end or !result) // fail if we did not get a full match
	{
		std::cout << "parse failure:\n";
		result = false;
	}

	return state;
}

}
