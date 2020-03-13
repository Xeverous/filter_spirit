#pragma once

#include <string>

namespace fs::version
{

constexpr int major = 0;
constexpr int minor = 5;
constexpr int patch = 0;

// C++20: add constexpr
inline std::string version_string()
{
	return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

}
