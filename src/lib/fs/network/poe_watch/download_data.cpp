#include <fs/network/poe_watch/download_data.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <future>
#include <utility>

namespace
{

constexpr auto target_name = "api.poe.watch";

}

namespace fs::network::poe_watch
{

std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	download_info* info,
	const log::monitor& logger)
{
	return std::async(std::launch::async, [](network_settings settings, download_info* info, const log::monitor& logger) {
		std::vector<std::string> urls = { "https://api.poe.watch/leagues" };
		download_result result = download(target_name, urls, std::move(settings), info, logger);
		return api_league_data{std::move(result.results[0].data)};
	}, std::move(settings), info, std::ref(logger));
}

api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const network_settings& settings,
	download_info* info,
	const log::monitor& logger)
{
	std::vector<std::string> urls = {
		"https://api.poe.watch/itemdata",
		"https://api.poe.watch/compact?league=" + url_encode(league_name)
	};

	download_result result = download(target_name, urls, settings, info, logger);

	return api_item_price_data {
		std::move(result.results[0].data),
		std::move(result.results[1].data)
	};
}

}
