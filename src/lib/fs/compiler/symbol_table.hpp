#pragma once

#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/parser/ast.hpp>
#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>

#include <string>
#include <unordered_map>

namespace fs::compiler {

struct named_object
{
	lang::object object_instance;
	lang::position_tag name_origin;
};

struct named_tree
{
	parser::ast::sf::statement_list_expression statements;
	lang::position_tag name_origin;
};

struct symbol_table
{
	[[nodiscard]] bool add_symbol( // defined in compiler.cpp
		settings st,
		const parser::ast::sf::constant_definition& def,
		diagnostics_container& diagnostics);

	[[nodiscard]] bool empty() const
	{
		return objects.empty() && trees.empty();
	}

	[[nodiscard]] std::size_t size() const
	{
		return objects.size() + trees.size();
	}

	std::unordered_map<std::string, named_object> objects;
	std::unordered_map<std::string, named_tree> trees;
};

}
