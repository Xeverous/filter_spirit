#include "fs/lang/types.hpp"
#include "fs/utility/visitor.hpp"

#include <type_traits>

namespace fs::lang
{

std::string_view to_string_view(object_type type)
{
	return type._name();
}

object_type type_of_object(const object& object)
{
	return std::visit(utility::visitor{
		[](const auto& value)
		{
			using T = std::decay_t<decltype(value)>;
			return type_to_enum<T>();
		}
	}, object.value);
}

}
