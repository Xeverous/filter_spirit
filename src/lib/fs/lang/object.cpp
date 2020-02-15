#include <fs/lang/object.hpp>
#include <fs/utility/visitor.hpp>
#include <fs/utility/type_traits.hpp>

#include <type_traits>

namespace fs::lang {

// these could be in the header but there is no need for them to be instantiated multiple times
static_assert(
	primitive_object_type::_size() == traits::variant_size_v<primitive_object_variant>,
	"there must be exactly one enum for each primitive_object variant type");

static_assert(
	object_type::_size() == traits::variant_size_v<object_variant>,
	"there must be exactly one enum for each any_object variant type");

object_type type_of_object(const object_variant& obj) noexcept
{
	return std::visit(utility::visitor{
		[](const sequence_object&) {
			return object_type::sequence;
		},
		[](const action_set&) {
			return object_type::compound_action;
		},
		[](query) {
			return object_type::query;
		}
	}, obj);
}

primitive_object_type type_of_primitive(const primitive_object_variant& obj) noexcept
{
	return std::visit(
		[](auto&& value) {
			using T = std::decay_t<decltype(value)>;
			return type_to_enum<T>();
		},
		obj);
}

inline bool operator==(const object& lhs, const object& rhs) noexcept
{
	// we intentionally do not compare origins
	// this operator is used by tests and potentially in the language
	return lhs.value == rhs.value;
}

}
