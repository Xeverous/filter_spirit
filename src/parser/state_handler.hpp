#pragma once
#include "parser/config.hpp"
#include "parser/ast.hpp"
#include "compiler/compiler.hpp"
#include "print/parse_error.hpp"
#include <cassert>
#include <utility>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fs::parser
{

using constants_map = std::unordered_map<std::string, lang::object>;

// note: the intended usage is to spam with string_view everywhere
// which all will be valid as long as object of this type is alive
class state_handler
{
public:
	explicit
	state_handler(
		std::string file_path,
		std::string file_content)
	: file_path(std::move(file_path)),
	  file_content(std::move(file_content)),
	  position_cache(
		  iterator_type(this->file_content.begin()),
		  iterator_type(this->file_content.end()))
	{
		assert(!this->file_content.empty());
	}

	// don't change the name of this function, it is expected by x3::annotate_on_success
	template <typename AST>
	void tag(AST& ast, iterator_type first, iterator_type last)
	{
		return position_cache.annotate(ast, first, last);
	}

	range_type position_of(x3::position_tagged pos) const
	{
		return position_cache.position_of(pos);
	}

	const position_cache_type& get_position_cache() const
	{
		return position_cache;
	}

	const std::string& get_file_content() const noexcept { return file_content; }

	ast::ast_type& get_ast() { return ast; }
	constants_map& get_map() { return map; }

	// TODO: this is too much of a god class, simplify coree state from errors
	void parse_error(x3::position_tagged pos, std::string_view message) /* const */
	{
		range_type where = position_cache.position_of(pos);
		print::parse_error(std::cout, get_range_of_all(), where, message);
	}

	void parse_error(iterator_type err_pos, std::string_view error_message) /* const */
	{
		print::parse_error(std::cout, get_range_of_all(), err_pos, error_message);
	}

	void compile_error(range_type err_pos, std::string_view error_message)
	{
		print::parse_error(std::cout, get_range_of_all(), err_pos, error_message);
	}

	void print_ast() const;
	void print_map() const;

private:
	range_type get_range_of_all() const
	{
		return range_type(position_cache.first(), position_cache.last());
	}

	const std::string file_path;
	const std::string file_content;
	position_cache_type position_cache;
	ast::ast_type ast;
	constants_map map;
};

}
