#pragma once

#include <fs/network/http.hpp>
#include <fs/log/logger_fwd.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <future>
#include <utility>

namespace fs::network
{

template <typename F>
auto async_download(
	const char* host,
	std::vector<std::string> targets,
	F response_handler, // signature: (std::vector<boost::beast::http::response<boost::beast::http::string_body>>) -> auto
	log::logger& logger)
{
	log_download_information(host, targets, logger);

	return std::async(std::launch::async, [host, targets = std::move(targets), f = std::move(response_handler)]()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		ctx.set_verify_mode(boost::asio::ssl::verify_none);
		auto request = async_http_get(ioc, ctx, host, std::move(targets));
		ioc.run();

		return f(request.get());
	});
}

}
