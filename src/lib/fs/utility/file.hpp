#pragma once

#include <fs/log/logger.hpp>

#include <boost/filesystem/path.hpp>

#include <system_error>
#include <string>
#include <string_view>

namespace fs::utility
{

// TODO logger overloads cause a small circular dependency between log and utility modules

[[nodiscard]] std::string
load_file(const boost::filesystem::path& path, std::error_code& ec);
[[nodiscard]] std::optional<std::string>
load_file(const boost::filesystem::path& path, log::logger& logger);

[[nodiscard]] std::error_code
save_file(const boost::filesystem::path& path, std::string_view file_contents);
[[nodiscard]] bool
save_file(const boost::filesystem::path& path, std::string_view file_contents, log::logger& logger);

}
