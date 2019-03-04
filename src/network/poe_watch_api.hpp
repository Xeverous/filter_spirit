#pragma once
#include "itemdata/types.hpp"
#include <vector>
#include <future>

namespace fs::network
{

class poe_watch_api
{
public:
	const std::vector<itemdata::league>& get_leagues() const noexcept { return leagues; }
	const itemdata::item_price_data& get_item_price_data() const noexcept { return item_price_data; }

	std::future<std::vector<itemdata::league>> async_download_leagues();
	std::future<itemdata::item_price_data> async_download_item_prices(int league_id);

	static constexpr auto target = "api.poe.watch";
	static constexpr auto port = "443";

private:
	std::vector<itemdata::league> leagues;
	itemdata::item_price_data item_price_data;
};

}
