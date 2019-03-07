#include "lang/types.hpp"
#include "lang/keywords.hpp"
#include "utility/visitor.hpp"
#include <cassert>
#include <variant>

namespace fs::lang
{

std::string_view to_string(single_object_type type)
{
	switch (type)
	{
		case single_object_type::boolean:
			return keywords::boolean;
		case single_object_type::integer:
			return keywords::integer;
		case single_object_type::floating_point:
			return keywords::floating_point;
		case single_object_type::level:
			return keywords::level;
		case single_object_type::quality:
			return keywords::quality;
		case single_object_type::font_size:
			return keywords::font_size;
		case single_object_type::sound_id:
			return keywords::sound_id;
		case single_object_type::volume:
			return keywords::volume;
		case single_object_type::socket_group:
			return keywords::socket_group;
		case single_object_type::rarity:
			return keywords::rarity_type;
		case single_object_type::shape:
			return keywords::shape;
		case single_object_type::suit:
			return keywords::suit;
		case single_object_type::color:
			return keywords::color;
		case single_object_type::minimap_icon:
			return keywords::minimap_icon;
		case single_object_type::beam_effect:
			return keywords::beam_effect;
		case single_object_type::string:
			return keywords::string;
		case single_object_type::path:
			return keywords::path;
		case single_object_type::alert_sound:
			return keywords::alert_sound;
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
	if (std::holds_alternative<integer>(obj))
		return single_object_type::integer;
	if (std::holds_alternative<floating_point>(obj))
		return single_object_type::floating_point;
	if (std::holds_alternative<level>(obj))
		return single_object_type::level;
	if (std::holds_alternative<quality>(obj))
		return single_object_type::quality;
	if (std::holds_alternative<font_size>(obj))
		return single_object_type::font_size;
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
	if (std::holds_alternative<socket_group>(obj))
		return single_object_type::socket_group;
	if (std::holds_alternative<string>(obj))
		return single_object_type::string;
	if (std::holds_alternative<path>(obj))
		return single_object_type::path;
	if (std::holds_alternative<alert_sound>(obj))
		return single_object_type::alert_sound;

	assert(false);
	// make a return to avoid UB
	// debug: fire assertion
	// release: return boolean as it is the most likely to be a noisy bug
	return single_object_type::boolean;
}

object_type type_of_object(const object& obj)
{
	return std::visit(utility::visitor{
		[](const single_object& so) { return type_of_object(so); },
		[](const array_object& ao)
		{
			if (ao.empty())
				return object_type(single_object_type::generic, true);

			const auto& inner_object_value = ao.front().value;
			assert(std::holds_alternative<single_object>(inner_object_value));
			return object_type(type_of_single_object(std::get<single_object>(inner_object_value)), true);
		}
	}, obj.value);
}

object_type type_of_object(const single_object& object)
{
	return object_type{type_of_single_object(object), false};
}

}
