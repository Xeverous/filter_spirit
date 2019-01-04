
#include "utility/utility.hpp"
#include "parser/ast_adapted.hpp"
#include "parser/grammar.hpp"

#include <iostream>

bool parse(std::string const& source)
{
	namespace x3 = boost::spirit::x3;

	fs::parser::iterator_type it = source.begin();
	const fs::parser::iterator_type end = source.end();
	fs::parser::grammar_type::attribute_type data;

	const bool result = x3::phrase_parse(
		it,
		end,
		fs::grammar(),
		fs::skipper(),
		data
	);

	if (it != end) // fail if we did not get a full match
	{
		std::cout << "parse failure at pos: " << it - source.begin() << "\n";
		return false;
	}

	return result;
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "usage: " << "./program filepath\n";
		return -1;
	}

	if (parse(fs::utility::load_file(argv[1]).value()))
	{
		std::cout << "parse successful\n";
	}
	else
	{
		std::cout << "parse failure\n";
	}
}
