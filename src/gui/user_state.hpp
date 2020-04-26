#pragma once

#include <string>

struct generation_state
{
	std::string filter_template_path;
	std::string output_directory;
	std::string generated_files_name_scheme;
	int selected_price_data = 0;
};

struct program_settings
{

};

struct user_state
{
	generation_state gen_state;
	program_settings settings;
};
