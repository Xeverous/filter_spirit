#pragma once
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include <string>
#include <iosfwd>
#include <optional>
#include <utility>
#include <unordered_map>

namespace fs::parser
{

using constants_map = std::unordered_map<std::string, lang::object>; // TODO move to compiler

struct parse_result
{
	parse_result(ast::ast_type ast, position_cache_type position_cache)
	: ast(std::move(ast)), position_cache(std::move(position_cache))
	{
	}

	void print_ast() const;

	[[nodiscard]]
	range_type get_range_of_all() const
	{
		return range_type(position_cache.first(), position_cache.last());
	}

	ast::ast_type ast;
	position_cache_type position_cache;
};

[[nodiscard]]
std::optional<parse_result> parse(const std::string& file_content, std::ostream& error_stream);

}
