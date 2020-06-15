#pragma once

#include <fs/log/monitor.hpp>

#include <filesystem>
#include <system_error>
#include <string>
#include <string_view>

namespace fs::utility
{

[[nodiscard]] std::string
load_file(const std::filesystem::path& path, std::error_code& ec);
[[nodiscard]] std::optional<std::string>
load_file(const std::filesystem::path& path, const log::monitor& logger);

[[nodiscard]] std::error_code
save_file(const std::filesystem::path& path, std::string_view file_contents);
[[nodiscard]] bool
save_file(const std::filesystem::path& path, std::string_view file_contents, const log::monitor& logger);

[[nodiscard]] bool
create_directories(const std::filesystem::path& dirpath, const log::monitor& logger);

}
