#pragma once
#include "fs/itemdata/types.hpp"
#include <vector>
#include <future>
#include <string>

namespace fs { class logger; }

namespace fs::network
{

namespace poe_watch_api
{
	[[nodiscard]]
	std::future<std::vector<itemdata::league>> async_download_leagues();
	[[nodiscard]]
	std::future<itemdata::item_price_data> async_download_item_price_data(std::string league_name, logger& logger);

	static constexpr auto target = "api.poe.watch";
	static constexpr auto port = "443";
}

}
