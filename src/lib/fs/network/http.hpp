#pragma once

#include <fs/log/logger_fwd.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <future>
#include <vector>
#include <string>

namespace fs::network
{
using result_type = std::vector<std::string>;

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
