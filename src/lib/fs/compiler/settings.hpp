#pragma once
#include <boost/optional.hpp>

namespace fs::compiler
{

struct error_handling_settings
{
	bool stop_on_error = false;
	bool treat_warnings_as_errors = false;
};

struct generation_settings
{
	boost::optional<int> min_opacity;
	boost::optional<int> max_opacity;
	boost::optional<int> min_font_size;
	boost::optional<int> max_font_size;
	boost::optional<int> min_volume;
	boost::optional<int> max_volume;
};

struct settings
{
	bool ruthless_mode = false;
	bool print_ast = false;
	error_handling_settings error_handling;
	generation_settings generation;
};

}
