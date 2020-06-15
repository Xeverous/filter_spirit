#pragma once

#include <fs/network/ggg/api_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/monitor.hpp>

#include <future>

namespace fs::network::ggg
{

[[nodiscard]] std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	download_info* info,
	const log::monitor& logger);

}
