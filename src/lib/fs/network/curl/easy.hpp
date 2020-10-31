#pragma once

#include <fs/network/curl/error.hpp>
#include <fs/utility/assert.hpp>

#include <curl/curl.h>

#include <cstring>
#include <optional>
#include <system_error>

namespace fs::network::curl
{

class string
{
public:
	string(char* str)
	: _str(str)
	{
		FS_ASSERT(_str != nullptr);
		_len = std::strlen(_str);
	}

	~string()
	{
		curl_free(_str);
	}

	const char* c_str() const noexcept
	{
		return _str;
	}

	std::size_t size() const noexcept
	{
		return _len;
	}

	std::string_view to_string_view() const noexcept
	{
		return std::string_view(_str, _len);
	}

private:
	char* _str;
	std::size_t _len;
};

using write_callback_fn = auto (char*, size_t, size_t, void*) -> size_t;
using progress_callback_fn = auto (void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t) -> int;

class easy_handle
{
public:
	easy_handle()
	{
		_handle = curl_easy_init();
		if (_handle == nullptr)
			throw std::runtime_error("could not initialize libcurl handle");
	}

	~easy_handle()
	{
		curl_easy_cleanup(_handle);
	}

	std::optional<string> url_encode(std::string_view sv) const
	{
		// no idea why curl's URL escaping requires a handle
		char* encoded = curl_easy_escape(_handle, sv.data(), sv.size());
		if (encoded == nullptr) {
			// throw std::runtime_error(std::string("url encoding of '").append(sv).append("' failed"));
			return std::nullopt;
		}

		return string(encoded);
	}

	void reset()
	{
		curl_easy_reset(_handle);
	}

	// ---- BEHAVIOR OPTIONS ----

	[[nodiscard]] std::error_code progress_meter(bool enable)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_NOPROGRESS, enable ? 0L : 1L),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code signals(bool enable)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_NOSIGNAL, enable ? 0L : 1L),
			curl_category()
		);
	}

	// ---- CALLBACK OPTIONS ----

	[[nodiscard]] std::error_code write_callback(write_callback_fn& fn)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, &fn),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code write_callback_data(void* pointer)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_WRITEDATA, pointer),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code xferinfo_callback(progress_callback_fn& fn)
	{
		if (auto ec = progress_meter(true); ec)
			return ec;

		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_XFERINFOFUNCTION, &fn),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code xferinfo_callback_data(void* pointer)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_XFERINFODATA, pointer),
			curl_category()
		);
	}

	// ---- NETWORK OPTIONS ----

	// Before version 7.17.0, strings were not copied.
	// Instead the user was forced keep them available until libcurl no longer needed them.
	[[nodiscard]] std::error_code url(const char* rfc3986_str)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_URL, rfc3986_str),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code proxy(const char* proxy_str)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_PROXY, proxy_str),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code proxy_port(long port)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_PROXYPORT, port),
			curl_category()
		);
	}

	// ---- HTTP OPTIONS ----

	[[nodiscard]] std::error_code follow_redirects(bool enable)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_FOLLOWLOCATION, enable ? 1L : 0L),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code max_redirects(long amount)
	{
		if (auto ec = follow_redirects(true); ec)
			return ec;

		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_MAXREDIRS, amount),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code user_agent(const char* str)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_USERAGENT, str),
			curl_category()
		);
	}

	// ---- CONNECTION OPTIONS ----

	[[nodiscard]] std::error_code timeout(long seconds)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_TIMEOUT, seconds),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code timeout_ms(long milliseconds)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_TIMEOUT_MS, milliseconds),
			curl_category()
		);
	}

	// ---- SSL and SECURITY OPTIONS ----

	[[nodiscard]] std::error_code ssl_cert(const char* filename)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_SSLCERT, filename),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code proxy_ssl_cert(const char* filename)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_PROXY_SSLCERT, filename),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code ssl_cert_type(const char* type)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_SSLCERTTYPE, type),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code proxy_ssl_cert_type(const char* type)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_PROXY_SSLCERTTYPE, type),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code ssl_verify_peer(bool enable)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYPEER, enable ? 1L : 0L),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code ssl_verify_host(bool enable)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYHOST, enable ? 2L : 0L),
			curl_category()
		);
	}

	[[nodiscard]] std::error_code ca_info(const char* path)
	{
		return std::error_code(
			curl_easy_setopt(_handle, CURLOPT_CAINFO, path),
			curl_category()
		);
	}

	// ---- the most important function ----

	[[nodiscard]] std::error_code perform()
	{
		return std::error_code(curl_easy_perform(_handle), curl_category());
	}

private:
	easy_handle(CURL* handle)
	: _handle(handle)
	{
		FS_ASSERT(_handle != nullptr);
	}

	CURL* _handle = nullptr;
};

}
