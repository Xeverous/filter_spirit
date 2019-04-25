#pragma once

#include "fs/parser/ast.hpp"
#include "fs/lang/types.hpp"
#include "fs/lang/constants_map.hpp"
#include "fs/compiler/error.hpp"
#include "fs/itemdata/types.hpp"

#include <variant>

namespace fs::compiler
{

[[nodiscard]]
std::variant<lang::socket_group, error::error_variant> construct_socket_group(
	const parser::ast::value_expression_list& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data);

[[nodiscard]]
std::variant<lang::color, error::error_variant> construct_color(
	const parser::ast::value_expression_list& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data);

[[nodiscard]]
std::variant<lang::path, error::error_variant> construct_path(
	const parser::ast::value_expression_list& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data);

[[nodiscard]]
std::variant<lang::minimap_icon, error::error_variant> construct_minimap_icon(
	const parser::ast::value_expression_list& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data);

[[nodiscard]]
std::variant<lang::beam_effect, error::error_variant> construct_beam_effect(
	const parser::ast::value_expression_list& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data);

}
