#include "utility/file.hpp"
#include "parser/state_handler.hpp"
#include "parser/parser.hpp"
#include "compiler/compiler.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "usage: " << "./program filepath\n";
		return -1;
	}

	const char* filepath = argv[1];
	std::optional<std::string> source = fs::utility::load_file(filepath);
	if (!source)
	{
		std::cout << "error loading file\n";
		return -1;
	}

	std::string& file_content = *source;

	bool result;
	fs::parser::state_handler state = fs::parser::parse(filepath, std::move(file_content), result);
	if (!result)
	{
		std::cout << "parse failure\n";
		return -1;
	}

	std::cout << "parse successful\n";
	state.print_ast();

	fs::compiler::error::error_type error = fs::compiler::parse_constants(state);
	if (!std::holds_alternative<fs::compiler::error::no_error>(error))
	{
		std::cout << "error building constants\n";
		return -1;
	}


	std::cout << "constants build successfully\n";
	state.print_map();
}
