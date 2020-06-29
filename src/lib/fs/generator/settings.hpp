#pragma once

#include <fs/compiler/settings.hpp>

namespace fs::generator
{

struct settings
{
	bool print_ast = false;
	compiler::settings compile_settings = {};
};

}
