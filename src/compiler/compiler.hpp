#pragma once
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include "lang/types.hpp"
#include <string>
#include <unordered_map>

namespace fs::compiler
{

using constants_map = std::unordered_map<std::string, lang::object>;

[[nodiscard]]
bool semantic_analysis(
	const parser::ast::ast_type& ast,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream);

[[nodiscard]]
std::optional<constants_map> parse_constants(
	const parser::ast::constant_definition_list& constants_list,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream);

[[nodiscard]]
std::optional<error::error_variant> add_constant_from_definition(
	const parser::ast::constant_definition& def,
	const parser::lookup_data& lookup_data,
	constants_map& map);

}
