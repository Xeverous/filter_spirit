#pragma once
#include <string>
#include <optional>
#include <variant>

namespace fs::lang
{

enum class object_type
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
	string
};

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

using object = std::variant<
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

}
