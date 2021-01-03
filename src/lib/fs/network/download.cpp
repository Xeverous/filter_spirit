#include <fs/network/download.hpp>

#ifndef __EMSCRIPTEN__
#include <fs/network/curl/easy.hpp>
#include <fs/version.hpp>
#include <curl/curl.h>
#endif

#include <utility>
#include <stdexcept>

namespace
{

using namespace fs;

#ifndef __EMSCRIPTEN__
// https://tools.ietf.org/html/rfc7231#section-5.5.3
std::string fs_user_agent()
{
	return "FilterSpirit/" + to_string(version::current())
		+ " (" + version::repository_link + ") " + curl_version();
}

std::size_t write_callback(char* data, std::size_t /* size */, std::size_t nmemb, void* userdata) noexcept
{
	auto& result = *reinterpret_cast<network::request_result*>(userdata);

	try {
		result.data.append(data, nmemb);
		return nmemb;
	}
	catch (const std::exception& e) {
		result.is_error = true;
		result.data = e.what();
	}
	catch (...) {
		result.is_error = true;
		result.data = "(unknown error)";
	}

	return 0; // error: accepted 0 bytes
}

int xferinfo_callback(
	void* userdata,
	curl_off_t dltotal,
	curl_off_t dlnow,
	curl_off_t /* ultotal */,
	curl_off_t /* ulnow */) noexcept
{
	auto& info = *reinterpret_cast<network::download_info*>(userdata);

	info.xfer_info.store(
		network::download_xfer_info{
			static_cast<std::size_t>(dltotal),
			static_cast<std::size_t>(dlnow)
		},
		std::memory_order::memory_order_release);

	return 0; // no error
}

void save_error(network::request_result& res, std::error_code ec)
{
	res.is_error = true;
	res.data = ec.message();
}

void save_error_to_all(std::vector<network::request_result>& results, std::error_code ec)
{
	for (auto& r : results)
		save_error(r, ec);
}

[[nodiscard]] bool
setup_download(
	network::curl::easy_handle& easy,
	const network::download_settings& settings,
	std::vector<network::request_result>& results)
{
	if (const auto ec = easy.write_callback(write_callback); ec) {
		save_error_to_all(results, ec);
		return false;
	}

	if (const auto ec = easy.follow_redirects(true); ec) {
		save_error_to_all(results, ec);
		return false;
	}

	if (const auto ec = easy.user_agent(fs_user_agent().c_str()); ec) {
		save_error_to_all(results, ec);
		return false;
	}

	if (const auto ec = easy.timeout_ms(settings.timeout_milliseconds); ec) {
		save_error_to_all(results, ec);
		return false;
	}

	if (const auto ec = easy.ssl_verify_peer(settings.ssl_verify_peer); ec) {
		save_error_to_all(results, ec);
		return false;
	}

	if (const auto ec = easy.ssl_verify_host(settings.ssl_verify_host); ec) {
		save_error_to_all(results, ec);
		return false;
	}

	if (!settings.ca_info_path.empty()) {
		if (const auto ec = easy.ca_info(settings.ca_info_path.c_str()); ec) {
			save_error_to_all(results, ec);
			return false;
		}
	}

	if (!settings.proxy.empty()) {
		if (const auto ec = easy.proxy(settings.proxy.c_str()); ec) {
			save_error_to_all(results, ec);
			return false;
		}
	}

	return true;
}

void
log_download(
	log::logger& logger,
	std::string_view target_url)
{
	logger.info() << "GET " << target_url << '\n';
}

network::download_result
download_using_curl(
	std::string_view target_name,
	const std::vector<std::string>& urls,
	const network::download_settings& settings,
	network::download_info* info,
	log::logger& logger)
{
	std::vector<network::request_result> results(urls.size());
	network::curl::easy_handle easy;

	if (!setup_download(easy, settings, results))
		return network::download_result{std::move(results)};

	if (info) {
		info->requests_total.store(urls.size(), std::memory_order::memory_order_release);

		if (auto ec = easy.xferinfo_callback(xferinfo_callback); ec) {
			save_error_to_all(results, ec);
			return network::download_result{std::move(results)};
		}

		if (auto ec = easy.xferinfo_callback_data(info); ec) {
			save_error_to_all(results, ec);
			return network::download_result{std::move(results)};
		}
	}

	for (std::size_t i = 0; i < urls.size(); ++i) {
		if (info)
			info->requests_complete.store(i, std::memory_order::memory_order_release);

		if (const auto ec = easy.write_callback_data(&results[i]); ec) {
			save_error(results[i], ec);
			continue;
		}

		log_download(logger, urls[i]);
		if (const auto ec = easy.url(urls[i].c_str()); ec) {
			save_error(results[i], ec);
			continue;
		}

		if (const auto ec = easy.perform(); ec) {
			save_error(results[i], ec);
			continue;
		}
	}

	if (info)
		info->requests_complete.store(urls.size(), std::memory_order::memory_order_release);

	if (results.size() != urls.size()) {
		throw std::logic_error("failure while downloading from " + std::string(target_name) +
			": expected " + std::to_string(urls.size()) + " files but got " + std::to_string(results.size()));
	}

	for (const auto& r : results) {
		if (r.is_error)
			throw std::runtime_error(r.data);
	}

	return network::download_result{std::move(results)};
}
#endif // !__EMSCRIPTEN__

} // namespace

namespace fs::network {

download_result
download(
	std::string_view target_name,
	const std::vector<std::string>& urls,
	const download_settings& settings,
	download_info* info,
	log::logger& logger)
{
#ifdef __EMSCRIPTEN__
	(void) target_name;
	(void) urls;
	(void) settings;
	(void) info;
	(void) logger;
	throw std::runtime_error("Network download not supported in Emscripten build");
#else
	return download_using_curl(target_name, urls, settings, info, logger);
#endif
}

}
