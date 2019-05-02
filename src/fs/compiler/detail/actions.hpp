#pragma once

#include "fs/parser/ast.hpp"
#include "fs/compiler/error.hpp"
#include "fs/lang/action_set.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/itemdata/types.hpp"

#include <optional>

namespace fs::compiler::detail
{

[[nodiscard]]
std::optional<error::error_variant> add_action(
	const parser::ast::action& action,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::action_set& action_set);

}
