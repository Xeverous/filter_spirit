#pragma once

#include <fs/network/ggg/api_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

namespace fs::network::ggg
{

[[nodiscard]] api_league_data
download_leagues(
	download_settings settings,
	download_info* info,
	log::logger& logger);

}
