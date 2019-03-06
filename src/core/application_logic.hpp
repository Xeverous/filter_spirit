#pragma once

#include "itemdata/data_storage.hpp"

#include <optional>
#include <string>

namespace fs::core
{

class application_logic
{
public:
	bool generate_filter(const std::string& league_name, const std::string& source_filepath, const std::string& output_filepath);

	void list_leagues();

private:
	itemdata::data_storage storage;
};

}
