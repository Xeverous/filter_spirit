#pragma once

#include "parser/ast.hpp"
#include "compiler/error.hpp"
#include "lang/action_set.hpp"
#include "lang/constants_map.hpp"
#include "itemdata/types.hpp"

#include <optional>

namespace fs::compiler
{

[[nodiscard]]
std::optional<error::error_variant> add_action(
	const parser::ast::action& action,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::action_set& action_set);

}
