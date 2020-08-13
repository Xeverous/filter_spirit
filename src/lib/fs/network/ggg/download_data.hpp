#pragma once

#include <fs/network/ggg/api_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <future>

namespace fs::network::ggg
{

[[nodiscard]] std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	download_info* info,
	log::logger& logger);

}
