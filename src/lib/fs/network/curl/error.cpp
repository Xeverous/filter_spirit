#include <fs/network/curl/error.hpp>

#include <curl/curl.h>

#include <system_error>

namespace
{

class _curl_category : public std::error_category
{
	const char* name() const noexcept override
	{
		return "curl";
	}

	std::string message(int condition) const override
	{
		return curl_easy_strerror(static_cast<CURLcode>(condition));
	}
};

}

namespace fs::network::curl
{

const std::error_category& curl_category()
{
	static _curl_category cat;
	return cat;
}

}
