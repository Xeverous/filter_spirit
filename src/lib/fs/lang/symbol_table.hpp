#pragma once
#include <fs/lang/types.hpp>
#include <unordered_map>
#include <string>

namespace fs::lang
{

struct named_object
{
	object object_instance;
	position_tag name_origin;
};

using symbol_table = std::unordered_map<std::string, named_object>;

}
