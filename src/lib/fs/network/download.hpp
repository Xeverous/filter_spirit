#pragma once

#include <fs/log/logger.hpp>

#include <atomic>
#include <future>
#include <string>
#include <string_view>
#include <vector>

namespace fs::network {

namespace strings {

constexpr auto ca_bundle_desc =
	"Use specified Certificate Authority bundle for SSL/TLS connections. "
	"By default FS uses a built-in bundle which is a copy of "
	"the certificates that are delivered with Mozilla Firefox. "
	"This option has no effect when you opt out of SSL verification.";

constexpr auto proxy_desc =
	"use PROXY for transfers, example values: \"http://my.proxy:80\","
	" \"https://user:password@proxyserver.com:3128\""
	" see https://curl.haxx.se/libcurl/c/CURLOPT_PROXY.html for"
	" more examples, documentation and environmental variables";

}

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
	std::atomic_size_t requests_complete = 0;
	std::atomic_size_t requests_total = 0;
};

struct download_settings
{
	static constexpr long timeout_default = 60'000;
	static constexpr long timeout_never   =      0;

	long timeout_milliseconds = timeout_default;
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
	const download_settings& settings,
	download_info* info,
	log::logger& logger);

}
