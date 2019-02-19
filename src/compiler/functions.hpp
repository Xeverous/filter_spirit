#pragma once
#include "parser/ast.hpp"
#include "lang/types.hpp"
#include "lang/constants_map.hpp"
#include "compiler/error.hpp"
#include <variant>
#include <vector>

namespace fs::compiler
{

[[nodiscard]]
std::variant<lang::color, error::error_variant> construct_color(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map);

[[nodiscard]]
std::variant<lang::socket_group, error::error_variant> construct_socket_group(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map);

[[nodiscard]]
std::variant<lang::minimap_icon, error::error_variant> construct_minimap_icon(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map);

[[nodiscard]]
std::variant<lang::beam_effect, error::error_variant> construct_beam_effect(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map);

}
