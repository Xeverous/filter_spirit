#include "fs/compiler/convertions.hpp"
#include <cassert>
#include <utility>

namespace fs::compiler
{

std::variant<std::vector<std::string>, error::error_variant> array_to_strings(
	lang::array_object array)
{
	std::vector<std::string> result;
	for (lang::object& obj : array)
	{
		assert(std::holds_alternative<lang::single_object>(obj.value));
		auto& single_object = std::get<lang::single_object>(obj.value);

		if (!std::holds_alternative<lang::string>(single_object))
			return error::type_mismatch{
				lang::object_type{lang::single_object_type::string},
				lang::type_of_object(single_object),
				obj.value_origin};

		auto& string = std::get<lang::string>(single_object);
		result.push_back(std::move(string.value));
	}

	return result;
}

}
