#include "network/poe_watch_api.hpp"
#include "network/network.hpp"
#include "itemdata/parse_json.hpp"
#include <future>
#include <boost/asio.hpp>

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

		const std::string& result = request.get().body();
		return itemdata::parse_league_info(std::string_view(result.c_str(), result.size()));
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

		const std::string& result_compact = request_compact.get().body();
		const std::string& result_itemdata = request_itemdata.get().body();
		return itemdata::parse_item_prices(
			std::string_view(result_compact.c_str(), result_compact.size()),
			std::string_view(result_itemdata.c_str(), result_itemdata.size()));
	});
}

}
