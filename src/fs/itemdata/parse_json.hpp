#pragma once

#include "fs/itemdata/types.hpp"

#include <string_view>
#include <vector>

namespace fs::log { class logger; }

namespace fs::itemdata
{

[[nodiscard]]
std::vector<league> parse_league_info(std::string_view league_json);
[[nodiscard]]
item_price_data parse_item_prices(std::string_view itemdata_json, std::string_view compact_json, log::logger& logger);

}
