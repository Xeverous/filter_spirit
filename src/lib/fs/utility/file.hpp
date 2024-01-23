#pragma once

#include <fs/log/logger.hpp>

#include <filesystem>
#include <optional>
#include <system_error>
#include <string>
#include <string_view>

namespace fs::utility
{

[[nodiscard]] std::string
load_file(const std::filesystem::path& path, std::error_code& ec);
[[nodiscard]] std::optional<std::string>
load_file(const std::filesystem::path& path, log::logger& logger);

[[nodiscard]] std::error_code
save_file(const std::filesystem::path& path, std::string_view file_contents);
[[nodiscard]] bool
save_file(const std::filesystem::path& path, std::string_view file_contents, log::logger& logger);

[[nodiscard]] bool
create_directories(const std::filesystem::path& dirpath, log::logger& logger);

}
