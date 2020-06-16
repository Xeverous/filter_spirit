#pragma once

#include <fs/log/logger.hpp>
#include <fs/log/monitor.hpp>

#include <atomic>
#include <future>
#include <string>
#include <string_view>
#include <vector>

namespace fs::network
{

struct request_result
{
	std::string data;
	bool is_error = false; // if true, data contains error message
};

struct download_result
{
	 std::vector<request_result> results;
};

struct download_xfer_info
{
	std::size_t expected_download_size = 0;
	std::size_t bytes_downloaded_so_far = 0;
};

struct download_info
{
	// updated real-time
	std::atomic<download_xfer_info> xfer_info = {};
	std::atomic<std::size_t> requests_complete = 0;
};

struct network_settings
{
	long timeout_milliseconds = 0; /** (no timeout) */
	bool ssl_verify_peer = true;
	bool ssl_verify_host = true;
	std::string ca_info_path = "certificates/cacert.pem";
	std::string proxy = {};
};

/**
 * @brief run a synchronous download
 * @param target_name used only for generated errors
 * @param urls array of targets
 * @param settings networking settings
 * @param info if non-null, will be updated as the download progresses
 * @return array of results, 1 for each target
 */
[[nodiscard]] download_result
download(
	std::string_view target_name,
	const std::vector<std::string>& urls,
	const network_settings& settings,
	download_info* info,
	const fs::log::monitor& logger);

}
