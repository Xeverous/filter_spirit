#pragma once

#include <optional>

namespace fs::lang
{

// TODO Should these really be a part of lang namespace?
// They are not a part of FS language.
// Item price APIs are more close to the network.
enum class data_source_type { none, poe_watch, poe_ninja };

std::string_view to_string(data_source_type data_source) noexcept;
std::optional<data_source_type> from_string(std::string_view data_source) noexcept;

}
