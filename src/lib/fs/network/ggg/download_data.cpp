#include <fs/network/ggg/download_data.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <future>
#include <utility>

namespace
{

constexpr auto leagues_url = "https://api.pathofexile.com/leagues";

}

namespace fs::network::ggg
{

std::future<api_league_data>
async_download_leagues(
	network_settings settings,
	download_info* info,
	log::logger& logger)
{
	log_download(leagues_url, logger);

	return std::async(std::launch::async, [](network_settings settings, download_info* info) {
		std::vector<std::string> urls = { leagues_url };
		download_result result = download("api.pathofexile.com", urls, settings, info);
		return api_league_data{std::move(result.results[0].data)};
	}, settings, info);
}

}
