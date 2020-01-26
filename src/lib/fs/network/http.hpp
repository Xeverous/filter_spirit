#pragma once

#include <fs/log/logger.hpp>

#include <boost/beast/http.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <future>
#include <vector>
#include <string>
#include <string_view>

namespace fs::network
{

std::string url_encode(std::string_view str);

// result passed as output parameter to reuse memory
void url_encode(std::string_view str, std::string& buffer);

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
