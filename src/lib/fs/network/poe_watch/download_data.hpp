#pragma once

#include <fs/network/poe_watch/api_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/monitor.hpp>

#include <future>
#include <string>

namespace fs::network::poe_watch
{

[[nodiscard]] std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	download_info* info,
	const log::monitor& logger);

[[nodiscard]] api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const network_settings& settings,
	download_info* info,
	const log::monitor& logger);

}
