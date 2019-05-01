#include "fs/lang/types.hpp"
#include "fs/utility/visitor.hpp"
#include "fs/utility/type_traits.hpp"

#include <type_traits>

namespace fs::lang
{

std::string_view to_string_view(object_type type)
{
	return type._to_string();
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

// this could be in the header but there is no need for it to be parsed multiple times
static_assert(
	object_type::_size() == traits::variant_size_v<object_variant>,
	"there must be exactly one enum for each object variant type");

}
