#pragma once

#include <optional>
#include <string_view>

namespace fs::lang
{

enum class data_source_type { none, poe_watch, poe_ninja };

const char* to_string(data_source_type data_source) noexcept;
std::optional<data_source_type> from_string(std::string_view data_source) noexcept;

}
