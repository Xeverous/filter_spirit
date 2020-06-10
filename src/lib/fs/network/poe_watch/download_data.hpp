#pragma once

#include <fs/network/poe_watch/api_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <future>
#include <string>

namespace fs::network::poe_watch
{

[[nodiscard]] std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	download_info* info,
	log::logger& logger);

[[nodiscard]] std::future<api_item_price_data>
async_download_item_price_data(
	std::string league_name,
	network_settings settings,
	download_info* info,
	log::logger& logger);

}
