#pragma once

#include "network/poe_watch_api.hpp"
#include <utility>
#include <string>

namespace fs::itemdata
{

class data_storage
{
public:
	[[nodiscard]]
	std::future<std::vector<itemdata::league>> async_download_leagues();
	[[nodiscard]]
	const std::vector<itemdata::league>& get_leagues() const noexcept { return leagues; }
	void set_leagues(std::vector<itemdata::league> data) { data = std::move(leagues); }

	[[nodiscard]]
	std::future<itemdata::item_price_data> async_download_item_price_data(std::string league_name);
	[[nodiscard]]
	const itemdata::item_price_data& get_item_price_data() const noexcept { return item_price_data; }

private:
	network::poe_watch_api api;
	std::vector<itemdata::league> leagues;
	itemdata::item_price_data item_price_data;
};

}
