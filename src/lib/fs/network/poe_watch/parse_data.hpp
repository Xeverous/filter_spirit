#pragma once

#include <fs/network/poe_watch/api_data.hpp>
#include <fs/lang/league.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/log/logger.hpp>

#include <string_view>
#include <vector>

namespace fs::network::poe_watch
{

[[nodiscard]] std::vector<lang::league>
parse_league_info(
	std::string_view league_json);

[[nodiscard]] lang::market::item_price_data
parse_item_price_data(
	const api_item_price_data& ipd,
	log::logger& logger);

}
