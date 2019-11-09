#pragma once

#include <string>
#include <string_view>

namespace fs::network
{

std::string url_encode(std::string_view str);

// result passed as output parameter to reuse memory
void url_encode(std::string_view str, std::string& buffer);

}
