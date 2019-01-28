#pragma once
#include "parser/config.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>

namespace fs::lang
{

enum class single_object_type
{
	boolean,
	number,
	level,
	sound_id,
	volume,
	rarity,
	shape,
	suit,
	color,
	group,
	string,

	generic // for generic constructs such as empty arrays
};

struct object_type
{
	object_type(single_object_type type, bool is_array = false)
	: type(type), is_array(is_array)
	{
	}

	single_object_type type;
	bool is_array;
};

std::string_view to_string(single_object_type type);
std::string to_string(object_type type);

enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
enum class suit { red, green, blue, white, brown, yellow };

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

struct sound_id
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
	std::optional<int> a;
};

struct group
{
	int r = 0;
	int g = 0;
	int b = 0;
	int w = 0;
};

struct string
{
	std::string value;
};

using single_object = std::variant<
	boolean,
	number,
	level,
	sound_id,
	volume,
	color,
	rarity,
	shape,
	suit,
	group,
	string
>;

using array_object = std::vector<single_object>;

struct object
{
	std::variant<single_object, array_object> value;
	// for printing errors about mismatched types
	// if object_ originated from a literal this should point at the literal
	fs::parser::range_type type_origin;
	// if object_ was referened from other object_ this should point at the use of
	// that object_ - in other words, point at expression which object_ was assigned from
	fs::parser::range_type value_origin;
	// for printing error name already exists
	// if object originated from a literal this should be empty
	std::optional<fs::parser::range_type> name_origin;
};

}
