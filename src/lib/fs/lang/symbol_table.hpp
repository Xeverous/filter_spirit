#pragma once

#include <fs/lang/object.hpp>
#include <fs/lang/position_tag.hpp>

#include <fs/parser/ast.hpp> // TODO circular dependency

#include <unordered_map>
#include <string>

namespace fs::lang
{

struct named_object
{
	object object_instance;
	position_tag name_origin;
};

struct named_tree
{
	parser::ast::sf::statement_list_expression statements;
	position_tag name_origin;
};

struct symbol_table
{
	std::unordered_map<std::string, named_object> objects;
	std::unordered_map<std::string, named_tree> trees;
};

}
