#pragma once
#include <future>
#include <string_view>
#include <boost/beast/http.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

namespace fs::network
{

	std::future<boost::beast::http::response<boost::beast::http::string_body>>
	async_http_get(
		boost::asio::io_context& ioc,
		boost::asio::ssl::context& ctx,
		const char* host,
		std::string_view port,
		std::string_view target);

}
