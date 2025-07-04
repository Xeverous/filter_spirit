#pragma once

#include <fs/lang/style_overrides.hpp>

namespace fs::compiler
{

struct error_handling_settings
{
	bool stop_on_error = false;
	bool treat_warnings_as_errors = false;
};

struct settings
{
	bool ruthless_mode = false;
	bool print_ast = false;
	error_handling_settings error_handling;
	lang::style_overrides overrides;
};

}
