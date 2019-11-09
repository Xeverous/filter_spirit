#include <nlohmann/json.hpp>

namespace fs::utility
{

// dump JSON to string, pretty-printed
[[nodiscard]]
nlohmann::json::string_t dump_json(const nlohmann::json& json);

}
