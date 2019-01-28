#include "lang/types.hpp"
#include "lang/constants.hpp"
#include <cassert>

namespace fs::lang
{

std::string_view to_string(single_object_type type)
{
	switch (type)
	{
		namespace ck = constants::keywords;

		case single_object_type::boolean:
			return ck::boolean;
		case single_object_type::number:
			return ck::number;
		case single_object_type::level:
			return ck::level;
		case single_object_type::sound_id:
			return ck::sound_id;
		case single_object_type::volume:
			return ck::volume;
		case single_object_type::rarity:
			return ck::rarity;
		case single_object_type::shape:
			return ck::shape;
		case single_object_type::suit:
			return ck::suit;
		case single_object_type::color:
			return ck::color;
		case single_object_type::group:
			return ck::group;
		case single_object_type::string:
			return ck::string;
		case single_object_type::generic:
			return "?";
	}

	assert(false);
	return "(unknown type)\nplease report a bug with attached minimal source that reproduces it";
}

std::string to_string(object_type type)
{
	std::string_view single_type = to_string(type.type);

	if (type.is_array)
		return std::string("Array<").append(single_type).append(">");
	else
		return std::string(single_type.data(), single_type.length());
}

}
