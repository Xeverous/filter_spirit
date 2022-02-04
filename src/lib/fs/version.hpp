#pragma once

#include <string>

namespace fs::version
{

constexpr auto repository_link = "github.com/Xeverous/filter_spirit";

struct version_triplet
{
	int major;
	int minor;
	int patch;
};

constexpr version_triplet current() noexcept
{
	return {1, 3, 2};
}

// C++20: use operator<=>
constexpr int compare(version_triplet lhs, version_triplet rhs) noexcept
{
	if (lhs.major < rhs.major)
		return -1;

	if (lhs.major > rhs.major)
		return 1;

	if (lhs.minor < rhs.minor)
		return -1;

	if (lhs.minor > rhs.minor)
		return 1;

	if (lhs.patch < rhs.patch)
		return -1;

	if (lhs.patch > rhs.patch)
		return 1;

	return 0;
}

constexpr bool operator==(version_triplet lhs, version_triplet rhs) noexcept { return compare(lhs, rhs) == 0; }
constexpr bool operator!=(version_triplet lhs, version_triplet rhs) noexcept { return compare(lhs, rhs) != 0; }
constexpr bool operator< (version_triplet lhs, version_triplet rhs) noexcept { return compare(lhs, rhs) <  0; }
constexpr bool operator> (version_triplet lhs, version_triplet rhs) noexcept { return compare(lhs, rhs) >  0; }
constexpr bool operator<=(version_triplet lhs, version_triplet rhs) noexcept { return compare(lhs, rhs) <= 0; }
constexpr bool operator>=(version_triplet lhs, version_triplet rhs) noexcept { return compare(lhs, rhs) >= 0; }

// C++20: add constexpr
inline std::string to_string(version_triplet v)
{
	return std::to_string(v.major) + "." + std::to_string(v.minor) + "." + std::to_string(v.patch);
}

}
