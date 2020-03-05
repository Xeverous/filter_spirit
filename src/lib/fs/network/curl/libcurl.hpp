#pragma once

#include <curl/curl.h>

namespace fs::network::curl
{

class [[maybe_unused]] libcurl
{
public:
	libcurl(long flags = CURL_GLOBAL_ALL)
	{
		curl_global_init(flags);
	}

	~libcurl()
	{
		curl_global_cleanup();
	}

	// so far non-moveable non-copyable
	libcurl(const libcurl& other) = delete;
	libcurl& operator=(libcurl other) = delete;
	libcurl(libcurl&& other) = delete;
	libcurl& operator=(libcurl&& other) = delete;
};

}
