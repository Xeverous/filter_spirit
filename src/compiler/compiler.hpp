#pragma once
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include <string_view>
#include <optional>
#include <iosfwd>

namespace fs::compiler
{

[[nodiscard]]
bool compile(const std::string& file_content, std::ostream& error_stream);

/**
 * @brief attempt to convert sequence of characters (eg RRGBW) to group object
 * @return empty if string was not valid
 */
[[nodiscard]]
std::optional<lang::group> identifier_to_group(std::string_view identifier);

}
