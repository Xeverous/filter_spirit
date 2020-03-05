#include <fs/network/poe_watch/download_data.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <future>
#include <utility>

namespace fs::network::poe_watch
{

std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	log::logger& /* logger */)
{
	// TODO add thread-safe logging

	return std::async(std::launch::async, [](network_settings settings) {
		std::vector<std::string> urls = { "https://api.poe.watch/leagues" };

		download_result result = download(urls, settings);
		if (result.results.size() != urls.size()) {
			throw std::logic_error("logic error: downloaded a different "
				"number of files from poe.watch: expected " + std::to_string(urls.size()) +
				" but got " + std::to_string(result.results.size()));
		}

		for (const auto& r : result.results) {
			if (r.is_error)
				throw std::runtime_error(r.data);
		}

		return api_league_data{std::move(result.results[0].data)};
	}, settings);
}

std::future<api_item_price_data>
async_download_item_price_data(
	std::string league_name,
	network_settings settings,
	log::logger& /* logger */)
{
	// TODO add thread-safe logging

	return std::async(std::launch::async, [](std::string league_name, network_settings settings) {
		std::vector<std::string> urls = {
			"https://api.poe.watch/itemdata",
			"https://api.poe.watch/compact?league=" + url_encode(league_name)
		};

		download_result result = download(urls, settings);
		if (result.results.size() != urls.size()) {
			throw std::logic_error("logic error: downloaded a different "
				"number of files from poe.watch: expected 2 but got " + std::to_string(urls.size()));
		}

		for (const auto& r : result.results) {
			if (r.is_error)
				throw std::runtime_error(r.data);
		}

		return api_item_price_data {
			std::move(result.results[0].data),
			std::move(result.results[1].data)
		};
	}, std::move(league_name), settings);
}

}
