#pragma once
#include <optional>
#include <string>

namespace fs::core
{

class application_logic
{
public:
	bool load_source_file(const std::string& filepath);
	bool generate_filter(const std::string& filepath);

private:
	std::optional<std::string> source_file_content;
};

}
