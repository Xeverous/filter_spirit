#pragma once

#include <fs/network/poe_ninja/api_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/monitor.hpp>

#include <string>

namespace fs::network::poe_ninja
{

[[nodiscard]] api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const network_settings& settings,
	download_info* info,
	const log::monitor& logger);

}
