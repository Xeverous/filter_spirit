#include "lang/type_names.hpp"
#include "lang/keywords.hpp"
#include <cassert>

namespace fs::lang
{

std::string_view to_string(single_object_type type)
{
	switch (type)
	{
		case single_object_type::boolean:
			return keywords::boolean;
		case single_object_type::number:
			return keywords::number;
		case single_object_type::level:
			return keywords::level;
		case single_object_type::sound_id:
			return keywords::sound_id;
		case single_object_type::volume:
			return keywords::volume;
		case single_object_type::rarity:
			return keywords::rarity_type;
		case single_object_type::shape:
			return keywords::shape;
		case single_object_type::suit:
			return keywords::suit;
		case single_object_type::color:
			return keywords::color;
		case single_object_type::group:
			return keywords::group;
		case single_object_type::string:
			return keywords::string;
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
