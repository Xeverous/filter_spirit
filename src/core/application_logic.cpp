#include "core/application_logic.hpp"
#include "utility/file.hpp"
#include "compiler/compiler.hpp"
#include <iostream>

namespace fs::core
{

bool application_logic::load_source_file(const std::string& filepath)
{
	source_file_content = fs::utility::load_file(filepath);
	return source_file_content.has_value();
}

bool application_logic::generate_filter(const std::string& filepath)
{
	if (!source_file_content)
		return false;

	return fs::compiler::process_input(*source_file_content, std::cout);
}

}
