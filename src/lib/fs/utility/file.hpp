#pragma once

#include <boost/filesystem/path.hpp>

#include <system_error>
#include <string>
#include <string_view>

namespace fs::utility
{

std::string load_file(const boost::filesystem::path& path, std::error_code& ec);

[[nodiscard]]
std::error_code save_file(const boost::filesystem::path& path, std::string_view file_contents);

}
