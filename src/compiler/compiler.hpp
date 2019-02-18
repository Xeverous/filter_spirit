#pragma once
#include <string>
#include <iosfwd>

namespace fs::compiler
{

[[nodiscard]]
bool process_input(const std::string& input, std::ostream& error_stream);

}
