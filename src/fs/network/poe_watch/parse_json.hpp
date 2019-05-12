#pragma once

#include "fs/lang/league.hpp"
#include "fs/lang/item_price_data.hpp"

#include <string_view>
#include <vector>

namespace fs::log { class logger; }

namespace fs::network::poe_watch
{

[[nodiscard]]
std::vector<lang::league> parse_league_info(std::string_view league_json);
[[nodiscard]]
lang::item_price_data parse_item_prices(std::string_view itemdata_json, std::string_view compact_json, log::logger& logger);

}
