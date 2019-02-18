#include "lang/types.hpp"
#include "utility/visitor.hpp"
#include <cassert>

namespace fs::lang
{

single_object_type type_of_single_object(const single_object& obj)
{
	if (std::holds_alternative<boolean>(obj))
		return single_object_type::boolean;
	if (std::holds_alternative<integer>(obj))
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
