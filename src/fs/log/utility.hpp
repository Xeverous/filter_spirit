#pragma once

#include <string_view>

namespace fs::log
{

[[nodiscard]] // supports any line break style
int count_lines(const char* first, const char* last);

[[nodiscard]]
std::string_view make_string_view(const char* first, const char* last);

}
