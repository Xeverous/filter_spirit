#pragma once

#include <fs/network/poe_ninja/api_data.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/log/logger.hpp>

namespace fs::network::poe_ninja
{

[[nodiscard]] lang::market::item_price_data parse_item_price_data(const api_item_price_data& jsons, log::logger& logger);

}
