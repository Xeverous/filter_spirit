#pragma once
#include "itemdata/types.hpp"
#include <vector>
#include <future>
#include <string>

namespace fs::network
{

class poe_watch_api
{
public:
	[[nodiscard]]
	std::future<std::vector<itemdata::league>> async_download_leagues();
	[[nodiscard]]
	std::future<itemdata::item_price_data> async_download_item_prices(std::string league_name);

	static constexpr auto target = "api.poe.watch";
	static constexpr auto port = "443";
};

}
