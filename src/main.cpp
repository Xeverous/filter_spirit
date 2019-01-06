
#include "utility/utility.hpp"
#include "parser/ast_adapted.hpp"
#include "parser/grammar.hpp"
#include "parser/parser.hpp"

#include <iostream>


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "usage: " << "./program filepath\n";
		return -1;
	}

	const auto source = fs::utility::load_file(argv[1]);
	if (!source)
	{
		std::cout << "error loading file\n";
		return -1;
	}

	auto& input = *source;
	fs::parser::position_cache positions{input.begin(), input.end()};

	bool result;
	const auto ast = fs::parser::parse(input, positions, result);
	if (result)
	{
		std::cout << "parse successful\n";
		fs::parser::print_ast(ast);
	}
	else
	{
		std::cout << "parse failure\n";
	}
}
