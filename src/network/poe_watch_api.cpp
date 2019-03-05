#include "network/poe_watch_api.hpp"
#include "network/network.hpp"
#include "itemdata/parse_json.hpp"
#include <future>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

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
			// TODO log it instead
			throw std::runtime_error("unable to parse received JSON of length " + std::to_string(result.size()) + ":\n" + result);
		}
	});
}

std::future<itemdata::item_price_data> async_download_item_prices(int league_id)
{
	return std::async(std::launch::async, []()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		ctx.set_verify_mode(boost::asio::ssl::verify_none);

		std::future<boost::beast::http::response<boost::beast::http::string_body>> request_compact =
			async_http_get(ioc, ctx, poe_watch_api::target, poe_watch_api::port, "/compact");

		std::future<boost::beast::http::response<boost::beast::http::string_body>> request_itemdata =
			async_http_get(ioc, ctx, poe_watch_api::target, poe_watch_api::port, "/itemdata");

		ioc.run();

		const auto response_compact = request_compact.get();
		const auto response_itemdata = request_itemdata.get();
		const std::string& result_compact = response_compact.body();
		const std::string& result_itemdata = response_itemdata.body();
		return itemdata::parse_item_prices(
			std::string_view(result_compact.c_str(), result_compact.size()),
			std::string_view(result_itemdata.c_str(), result_itemdata.size()));
	});
}

}
