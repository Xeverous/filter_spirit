#include <fs/lang/object.hpp>
#include <fs/utility/type_traits.hpp>

namespace fs::lang {

// this could be in the header but there is no need for it to be instantiated multiple times
static_assert(
	object_type::_size() == traits::variant_size_v<object_variant>,
	"there must be exactly one enum for each object_variant variant type");

object_type object_type_of(const object_variant& obj) noexcept
{
	return object_type::_from_index(obj.index());
}

bool operator==(const object& lhs, const object& rhs) noexcept
{
	// we intentionally do not compare origins
	// this operator is used by tests and potentially in the language
	return lhs.values == rhs.values;
}

}
