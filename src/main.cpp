
#include "utility/utility.hpp"
#include "parser/grammar.hpp"
#include <boost/spirit/home/x3.hpp>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <sstream>

bool parse(std::string const& source)
{
	namespace x3 = boost::spirit::x3;

	using x3::double_;
	using x3::phrase_parse;

	auto it = source.begin();

	const bool result = phrase_parse(
		it,
		source.end(),
		*fs::parser::code_line,
		fs::parser::whitespace
	);

	if (it != source.end()) // fail if we did not get a full match
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
