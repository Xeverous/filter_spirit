#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/network/poe_ninja/api_data_adapted.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <boost/preprocessor/repeat.hpp>

#include <string_view>
#include <utility>

namespace fs::network::poe_ninja {

namespace {

std::string poe1_make_url(bool is_currency, std::string_view league_encoded, std::string_view name)
{
	std::string result;
	result.reserve(127);

	if (is_currency)
		result = "https://poe.ninja/poe1/api/economy/exchange/current/overview";
	else
		result = "https://poe.ninja/poe1/api/economy/stash/current/item/overview";

	// apply league parameter first - looks nicer in logs (common string prefix)
	result.append("?league=").append(league_encoded);
	result.append("&type=").append(name);

	return result;
}

std::string poe2_make_url(std::string_view league_encoded, std::string_view category)
{
	std::string result = "https://poe.ninja/poe2/api/economy/exchange/current/overview";

	result.append("?league=").append(league_encoded);
	result.append("&type=").append(category);

	return result;
}

} // namespace

namespace poe1 {

api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const download_settings& settings,
	download_info* info,
	log::logger& logger)
{
	const std::string league_encoded = url_encode(league_name);

	api_item_price_data data;
	std::vector<std::string> urls;
	data.for_each_file([&](const char* name, bool is_currency, std::string& /* content */) {
		urls.push_back(poe1_make_url(is_currency, league_encoded, name));
	});

	download_result result = download("poe.ninja/poe1/api", urls, settings, info, logger);

	if (result.results.size() != static_cast<unsigned>(data.num_files()))
	{
		logger.error() << "failed to download JSON data files";
		return data;
	}

	std::size_t i = 0;
	data.for_each_file([&](const char* /* name */, bool /* is_currency */, std::string& content) {
		if (!result.results[i].is_error)
			content = std::move(result.results[i++].data);
	});

	return data;
}

} // namespace poe1

namespace poe2 {

api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const download_settings& settings,
	download_info* info,
	log::logger& logger)
{
	const std::string league_encoded = url_encode(league_name);

	api_item_price_data data;
	std::vector<std::string> urls;
	data.for_each_file([&](const char* category, std::string& /* content */) {
		urls.push_back(poe2_make_url(league_encoded, category));
	});

	download_result result = download("poe.ninja/poe2/api", urls, settings, info, logger);

	if (result.results.size() != static_cast<unsigned>(data.num_files()))
	{
		logger.error() << "failed to download JSON data files";
		return data;
	}

	std::size_t i = 0;
	data.for_each_file([&](const char* /* name */, std::string& content) {
		if (!result.results[i].is_error)
			content = std::move(result.results[i++].data);
	});

	return data;
}

} // namespace poe2

}
