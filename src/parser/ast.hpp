/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

namespace fs::ast
{

namespace x3 = boost::spirit::x3;

struct constant_boolean_definition : x3::position_tagged
{
	std::string name;
	bool value;
};

struct constant_number_definition : x3::position_tagged
{
	std::string name;
	int value;
};

struct constant_level_definition : x3::position_tagged
{
	std::string name;
	int value;
};

struct constant_sound_id_definition : x3::position_tagged
{
	std::string name;
	int value;
};

struct constant_volume_definition : x3::position_tagged
{
	std::string name;
	int value;
};

//struct constant_definition : x3::position_tagged
//{
//	std::string name;
//	std::variant<?> value;
//};

struct level
{
	int value;
};

struct volume_id
{
	int value;
};

struct volume
{
	int value;
};

struct color
{
	int r;
	int g;
	int b;
	int a;
};

struct suit
{
	// enum?
};

struct shape
{
	// enum?
};

struct rarity
{
	// enum?
};

struct group
{
	int r;
	int g;
	int b;
	int w;
};

}
