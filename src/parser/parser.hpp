#pragma once
#include "parser/state_handler.hpp"
#include <string>

namespace fs::parser
{

[[nodiscard]]
state_handler parse(std::string filepath, std::string file_content, bool& result);

}
