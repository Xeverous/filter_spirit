#pragma once

namespace fs::compiler
{

struct error_handling_settings
{
	bool stop_on_error = false;
	bool treat_warnings_as_errors = false;
};

struct settings
{
	error_handling_settings error_handling;
	int max_recursion_depth = 128;
};

}
