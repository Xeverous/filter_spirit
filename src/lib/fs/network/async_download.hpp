#pragma once

#include <fs/network/http.hpp>
#include <fs/log/logger.hpp>

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
	F response_handler, // signature: (fs::network::result_type) -> auto
	log::logger& logger)
{
	log_download_information(host, targets, logger);

	return std::async(std::launch::async, [host, targets = std::move(targets), f = std::move(response_handler)]()
	{
		boost::asio::io_context ioc;
		boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
		// No SSL certificate checking as of now (not so trivial to implement).
		// This potentially puts us into being able to be fooled while reacing
		// some other target falsely claiming to be our desired one but since
		// we do not send any sensitive data we only risk getting bad responses
		ctx.set_verify_mode(boost::asio::ssl::verify_none);
		auto request = async_http_get(ioc, ctx, host, std::move(targets));
		ioc.run();

		return f(request.get());
	});
}

}
