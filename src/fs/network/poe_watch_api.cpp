#include "fs/network/poe_watch_api.hpp"
#include "fs/network/network.hpp"
#include "fs/itemdata/parse_json.hpp"
#include "fs/log/logger.hpp"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <future>
#include <utility>

namespace
{

std::pair<unsigned char, unsigned char> hexchar(unsigned char c)
{
	unsigned char hex1 = c / 16;
	unsigned char hex2 = c % 16;
	hex1 += hex1 <= 9 ? '0' : 'a' - 10;
	hex2 += hex2 <= 9 ? '0' : 'a' - 10;
	return {hex1, hex2};
}

std::string url_encode(std::string_view str)
{
	std::string result;
	result.reserve(str.size() + str.size() / 2);
	for (char c : str)
	{
		if ((c >= '0' && c <= '9') ||
		        (c >= 'a' && c <= 'z') ||
		        (c >= 'A' && c <= 'Z') ||
		        c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
		        c == '*' || c == '\'' || c == '(' || c == ')')
		{
			result.push_back(c);
		}
		else if (c == ' ')
		{
			result.push_back('+');
		}
		else
		{
			result.push_back('%');
			const auto pair = hexchar(c);
			result.push_back(pair.first);
			result.push_back(pair.second);
		}
	}

	return result;
}

std::string url_encode_param(std::string_view param_name, std::string_view param_value)
{
	return url_encode(param_name) + "=" + url_encode(param_value);
}

} // namespace

namespace fs::network
{

std::future<std::vector<itemdata::league>> poe_watch_api::async_download_leagues()
{
	return std::async(std::launch::async, []()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		ctx.set_verify_mode(boost::asio::ssl::verify_none);

		std::future<boost::beast::http::response<boost::beast::http::string_body>> request =
			async_http_get(ioc, ctx, poe_watch_api::target, poe_watch_api::port, "/leagues");

		ioc.run();

		const auto response = request.get();
		const std::string& result = response.body();

		try {
			return itemdata::parse_league_info(std::string_view(result.c_str(), result.size()));
		}
		catch (const nlohmann::json::parse_error& error)
		{
			throw std::runtime_error("unable to parse received JSON of length " + std::to_string(result.size()) + ":\n" + result);
		}
	});
}

std::future<itemdata::item_price_data> poe_watch_api::async_download_item_price_data(std::string league_name, log::logger& logger)
{
	return std::async(std::launch::async, [&logger, league = std::move(league_name)]()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		ctx.set_verify_mode(boost::asio::ssl::verify_none);

		const std::string compact_query = "/compact?" + url_encode_param("league", league);
		std::future<boost::beast::http::response<boost::beast::http::string_body>> request_compact =
			async_http_get(ioc, ctx, poe_watch_api::target, poe_watch_api::port, compact_query);

		const std::string itemdata_query = "/itemdata";
		std::future<boost::beast::http::response<boost::beast::http::string_body>> request_itemdata =
			async_http_get(ioc, ctx, poe_watch_api::target, poe_watch_api::port, itemdata_query);

		logger.info() << "querying: " << poe_watch_api::target << compact_query;
		logger.info() << "querying: " << poe_watch_api::target << itemdata_query;
		ioc.run();

		const auto response_compact = request_compact.get();
		const auto response_itemdata = request_itemdata.get();
		const std::string& result_compact = response_compact.body();
		const std::string& result_itemdata = response_itemdata.body();
		return itemdata::parse_item_prices(
			std::string_view(result_itemdata.c_str(), result_itemdata.size()),
			std::string_view(result_compact.c_str(), result_compact.size()),
			logger);
	});
}

}
