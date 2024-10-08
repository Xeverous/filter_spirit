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
	download_settings settings,
	download_info* info,
	log::logger& logger);

[[nodiscard]] api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const download_settings& settings,
	download_info* info,
	log::logger& logger);

}
