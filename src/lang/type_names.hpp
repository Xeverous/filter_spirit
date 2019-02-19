#pragma once
#include <string>
#include <string_view>

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
	socket_group,
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

[[nodiscard]]
std::string_view to_string(single_object_type type);
[[nodiscard]]
std::string to_string(object_type type);

}
