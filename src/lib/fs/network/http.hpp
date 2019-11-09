#pragma once

#include <fs/log/logger_fwd.hpp>

#include <boost/beast/http.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <future>
#include <vector>
#include <string>

namespace fs::network
{

[[nodiscard]] std::future<std::vector<boost::beast::http::response<boost::beast::http::string_body>>>
async_http_get(
	boost::asio::io_context& ioc,
	boost::asio::ssl::context& ctx,
	const char* host,
	std::vector<std::string> targets);

void log_download_information(
	const char* host,
	const std::vector<std::string>& targets,
	log::logger& logger);

}
