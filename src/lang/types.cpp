#include "lang/types.hpp"
#include "lang/constants.hpp"
#include "utility/visitor.hpp"
#include <cassert>

namespace fs::lang
{

action_set action_set::override_by(const action_set& other) const
{
	action_set result = *this;

	if (other.show)
		result.show = other.show;
	if (other.border_color)
		result.border_color = other.border_color;
	if (other.text_color)
		result.text_color = other.text_color;
	if (other.background_color)
		result.background_color = other.background_color;
	if (other.font_size)
		result.font_size = other.font_size;
	if (other.sound)
		result.sound = other.sound;
	if (other.disabled_drop_sound)
		result.disabled_drop_sound = other.disabled_drop_sound;
	if (other.icon)
		result.icon = other.icon;
	if (other.effect)
		result.effect = other.effect;

	return result;
}

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
			return ck::rarity_type;
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

single_object_type type_of_single_object(const single_object& obj)
{
	if (std::holds_alternative<boolean>(obj))
		return single_object_type::boolean;
	if (std::holds_alternative<number>(obj))
		return single_object_type::number;
	if (std::holds_alternative<level>(obj))
		return single_object_type::level;
	if (std::holds_alternative<sound_id>(obj))
		return single_object_type::sound_id;
	if (std::holds_alternative<volume>(obj))
		return single_object_type::volume;
	if (std::holds_alternative<rarity>(obj))
		return single_object_type::rarity;
	if (std::holds_alternative<shape>(obj))
		return single_object_type::shape;
	if (std::holds_alternative<suit>(obj))
		return single_object_type::suit;
	if (std::holds_alternative<color>(obj))
		return single_object_type::color;
	if (std::holds_alternative<group>(obj))
		return single_object_type::group;
	if (std::holds_alternative<string>(obj))
		return single_object_type::string;

	assert(false);
	// make a return to avoid UB
	// debug: fire assertion
	// release: return boolean as it is the most likely to be a noisy bug
	return single_object_type::boolean;
}

object_type type_of_object(const object& obj)
{
	return std::visit(utility::visitor{
		[](const single_object& obj) { return object_type(type_of_single_object(obj)); },
		[](const array_object& obj)
		{
			if (obj.empty())
				return object_type(single_object_type::generic, true);

			return object_type(type_of_single_object(obj.front()), true);
		}
	}, obj.value);
}

}
