#pragma once

#include <fs/log/logger.hpp>

#include <atomic>
#include <future>
#include <string>
#include <vector>

namespace fs::network
{

struct http_result
{
	std::string data;
	bool is_error = false; // if true, data contains error message
};

struct download_result
{
	 std::vector<http_result> results;
};

struct download_xfer_info
{
	std::size_t expected_download_size = 0;
	std::size_t bytes_downloaded_so_far = 0;
};

struct async_download_info
{
	// updated real-time
	std::atomic<download_xfer_info> xfer_info;
	std::atomic<std::size_t> requests_complete = 0;
};

struct network_settings
{
	long timeout_milliseconds = 0; /** (no timeout) */
	bool ssl_verify_peer = true;
	bool ssl_verify_host = true;
	const char* ca_info_path = nullptr;
	const char* proxy = nullptr;
};

[[nodiscard]] download_result
download(
	const std::vector<std::string>& urls,
	network_settings settings);

[[nodiscard]] std::future<download_result>
async_dowload(
	std::vector<std::string> urls, // taken by value to be moved to a separate thread
	network_settings settings,
	async_download_info& info);

}
