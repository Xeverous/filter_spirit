#include "fs/network/poe_watch/api.hpp"
#include "fs/network/poe_watch/parse_json.hpp"
#include "fs/network/network.hpp"
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

std::future<boost::beast::http::response<boost::beast::http::string_body>> download(
	boost::asio::io_context& ioc,
	boost::asio::ssl::context& ctx,
	std::string_view target,
	fs::log::logger& logger)
{
	constexpr auto host = "api.poe.watch";
	logger.info() << "downloading from: " << host << target;
	return fs::network::async_http_get(ioc, ctx, host, "443", target);
}

} // namespace

namespace fs::network::poe_watch
{

std::future<std::string> async_download_leagues(log::logger& logger)
{
	return std::async(std::launch::async, [&]()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		ctx.set_verify_mode(boost::asio::ssl::verify_none);

		std::future<boost::beast::http::response<boost::beast::http::string_body>> request =
			download(ioc, ctx, "/leagues", logger);

		ioc.run();

		auto response = request.get();
		return std::move(response.body());
	});
}

std::future<item_price_data> async_download_item_price_data(std::string league_name, log::logger& logger)
{
	return std::async(std::launch::async, [&logger, league = std::move(league_name)]()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		ctx.set_verify_mode(boost::asio::ssl::verify_none);

		const std::string compact_query = "/compact?" + url_encode_param("league", league);
		std::future<boost::beast::http::response<boost::beast::http::string_body>> request_compact =
			download(ioc, ctx, compact_query, logger);

		const std::string itemdata_query = "/itemdata";
		std::future<boost::beast::http::response<boost::beast::http::string_body>> request_itemdata =
			download(ioc, ctx, itemdata_query, logger);

		ioc.run();

		auto response_compact = request_compact.get();
		auto response_itemdata = request_itemdata.get();
		return item_price_data{std::move(response_compact.body()), std::move(response_itemdata.body())};
	});
}

template <typename F>
auto invoke_wrap_exceptions(F f)
{
	try {
		return f();
	}
	catch (const nlohmann::json::parse_error& error)
	{
		throw std::runtime_error(std::string("unable to parse received JSON: ") + error.what());
	}
}

std::vector<lang::league> parse_leagues(std::string_view leagues_json)
{
	return invoke_wrap_exceptions([&]() {
		return parse_league_info(leagues_json);
	});
}

lang::item_price_data parse_item_price_data(const item_price_data& data, log::logger& logger)
{
	return invoke_wrap_exceptions([&]() {
		return parse_item_prices(data.itemdata_json, data.compact_json, logger);
	});
}

}
