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
		if (!std::holds_alternative<lang::string>(obj.value))
			return error::type_mismatch{
				lang::object_type::string,
				lang::type_of_object(obj),
				obj.value_origin};

		auto& string = std::get<lang::string>(obj.value);
		result.push_back(std::move(string.value));
	}

	return result;
}

}
