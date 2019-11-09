#include <fs/network/poe_watch/download_data.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/async_download.hpp>
#include <fs/log/logger.hpp>

#include <future>
#include <utility>

namespace
{

constexpr auto host = "api.poe.watch";

}

namespace fs::network::poe_watch
{

std::future<api_league_data> async_download_leagues(log::logger& logger)
{
	std::vector<std::string> targets = { "/leagues" };

	auto response_handler = [](std::vector<boost::beast::http::response<boost::beast::http::string_body>> responses) {
		if (responses.size() != 1u) {
			throw std::logic_error("logic error: downloaded a different "
				"number of files from poe.watch: expected 1 but got " + std::to_string(responses.size()));
		}

		return api_league_data{ std::move(responses[0]).body() };
	};

	return async_download(host, std::move(targets), response_handler, logger);
}

std::future<api_item_price_data> async_download_item_price_data(std::string league_name, log::logger& logger)
{
	std::vector<std::string> targets = {
		"/itemdata",
		"/compact?league=" + url_encode(league_name)
	};

	auto response_handler = [league = std::move(league_name)](std::vector<boost::beast::http::response<boost::beast::http::string_body>> responses) {
		if (responses.size() != 2u) {
			throw std::logic_error("logic error: downloaded a different "
				"number of files from poe.watch: expected 2 but got " + std::to_string(responses.size()));
		}

		return api_item_price_data{
			std::move(responses[0]).body(),
			std::move(responses[1]).body(),
		};
	};

	return async_download(host, std::move(targets), response_handler, logger);
}

}
