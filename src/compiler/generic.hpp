#pragma once
#include "compiler/compiler.hpp"
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include "lang/constants_map.hpp"
#include "lang/condition_set.hpp"
#include "lang/action_set.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include <string_view>
#include <optional>

namespace fs::compiler
{

/**
 * @brief attempt to convert sequence of characters (eg RRGBW) to a group
 * @return empty if string was not valid
 */
[[nodiscard]]
std::optional<lang::group> identifier_to_group(std::string_view identifier);

[[nodiscard]]
std::variant<lang::action_set, error::error_variant> construct_action_set(
	const std::vector<parser::ast::action>& actions,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
std::variant<lang::condition_set, error::error_variant> construct_condition_set(
	const std::vector<parser::ast::condition>& conditions,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data);

}
