#include "lang/types.hpp"
#include "lang/constants.hpp"
#include <cassert>

namespace fs::lang
{

std::string_view to_string(object_type type)
{
	switch (type)
	{
		namespace ck = constants::keywords;

		case object_type::boolean:
			return ck::boolean;
		case object_type::number:
			return ck::number;
		case object_type::level:
			return ck::level;
		case object_type::sound_id:
			return ck::sound_id;
		case object_type::volume:
			return ck::volume;
		case object_type::rarity:
			return ck::rarity;
		case object_type::shape:
			return ck::shape;
		case object_type::suit:
			return ck::suit;
		case object_type::color:
			return ck::color;
		case object_type::group:
			return ck::group;
		case object_type::string:
			return ck::string;
	}

	assert(false);
	return "(unknown type)\nplease report a bug with attached minimal source that reproduces it";
}

}
