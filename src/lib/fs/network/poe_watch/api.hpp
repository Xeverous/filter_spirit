#pragma once

#include <fs/lang/item_price_data.hpp>
#include <fs/lang/league.hpp>

#include <vector>
#include <future>
#include <string>

namespace fs::log { class logger; }

namespace fs::network::poe_watch
{

struct item_price_data
{
	// https://poe.watch/api
	std::string compact_json;
	std::string itemdata_json;
};

[[nodiscard]]
std::future<std::string> async_download_leagues(log::logger& logger);
[[nodiscard]]
std::vector<lang::league> parse_leagues(std::string_view leagues_json);

[[nodiscard]]
std::future<item_price_data> async_download_item_price_data(std::string league_name, log::logger& logger);
[[nodiscard]]
lang::item_price_data parse_item_price_data(const item_price_data& data, log::logger& logger);

}
