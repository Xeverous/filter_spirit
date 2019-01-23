#pragma once
#include "parser/state_handler.hpp"
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include <string_view>
#include <iosfwd>

namespace fs::compiler
{

[[nodiscard]]
bool compile(std::string file_content, std::string_view output_path, std::ostream& error_stream);

[[nodiscard]]
bool parse_constants(parser::state_handler& state);

/**
 * @brief attempt to convert sequence of characters (eg RRGBW) to group object
 * @return empty if string was not valid
 */
[[nodiscard]]
std::optional<lang::group> identifier_to_group(std::string_view identifier);

}
