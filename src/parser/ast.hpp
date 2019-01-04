/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once

#include <boost/spirit/home/x3.hpp>

namespace fs::ast
{

struct boolean
{
	bool value;
};

struct number
{
	int value;
};

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
