#pragma once

#include <curl/curl.h>

#include <system_error>

namespace fs::network::curl
{

const std::error_category& curl_category();

}

namespace std
{

template <>
struct is_error_code_enum<CURLcode> : std::true_type {};

}
