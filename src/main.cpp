#include "compiler/process_input.hpp"
#include "utility/file.hpp"
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
	bool result = fs::compiler::process_input(std::move(file_content), std::cout);
	return !result;
}
