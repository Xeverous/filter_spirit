#include <fs/utility/dump_json.hpp>

namespace fs::utility
{

nlohmann::json::string_t dump_json(const nlohmann::json& json)
{
	return json.dump(
		4,                                         // indent
		' ',                                       // indentation character
		true,                                      // escape non-ASCII characters as \xXXXX
		nlohmann::json::error_handler_t::replace); // replace invalid UTF-8 sequences with U+FFFD
}

}
