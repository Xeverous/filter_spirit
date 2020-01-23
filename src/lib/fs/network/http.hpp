#pragma once

#include <fs/log/logger.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <future>
#include <vector>
#include <string>
#include <string_view>

namespace fs::network
{

using result_type = std::vector<std::string>;

std::string url_encode(std::string_view str);

// result passed as output parameter to reuse memory
void url_encode(std::string_view str, std::string& buffer);

[[nodiscard]] std::future<result_type>
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
