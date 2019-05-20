#pragma once
#include <string>
#include <optional>

namespace fs::utility
{

// note: do not provide add string_view overload
// because OS API expect null-terminated strings
bool file_exists(const char* path);
inline
bool file_exists(const std::string& path) { return file_exists(path.c_str()); }

std::optional<std::string> load_file(const char* file_path);
inline
std::optional<std::string> load_file(const std::string& file_path) { return load_file(file_path.c_str()); }

}