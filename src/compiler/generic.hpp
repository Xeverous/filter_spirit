#pragma once
#include "lang/types.hpp"
#include <string_view>
#include <optional>

// forward declare to avoid too heavy includes
namespace fs::parser::ast { class value_expression; }

namespace fs::compiler
{

/**
 * @brief attempt to convert sequence of characters (eg RRGBW) to group object
 * @return empty if string was not valid
 */
[[nodiscard]]
std::optional<lang::group> identifier_to_group(std::string_view identifier);

// get type's enum from object - mostly for error printing purposes
[[nodiscard]]
fs::lang::object_type type_of_object(const fs::lang::object& obj);

/**
 *
 * @param expression denoting a literal, CAN NOT be an identifier expression
 * @return expresion as a language object
 */
[[nodiscard]] // C++20: add [[expects]] for the literal
fs::lang::object parser_literal_to_language_object(const fs::parser::ast::value_expression& literal);

}
