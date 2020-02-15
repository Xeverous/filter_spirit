#pragma once

#include <tuple>
#include <string>
#include <variant>

namespace fs::lang
{

struct none {};

inline bool operator==(none, none) noexcept { return true; }
inline bool operator!=(none, none) noexcept { return false; }

struct underscore {};

inline bool operator==(underscore, underscore) noexcept { return true; }
inline bool operator!=(underscore, underscore) noexcept { return false; }

struct boolean
{
	bool value;
};

inline bool operator==(boolean lhs, boolean rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(boolean lhs, boolean rhs) noexcept { return !(lhs == rhs); }

struct integer
{
	int value;
};

inline bool operator==(integer lhs, integer rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(integer lhs, integer rhs) noexcept { return !(lhs == rhs); }

struct floating_point
{
	explicit floating_point(double value) : value(value) {}
	explicit floating_point(integer n) : value(n.value) {}

	double value;
};

inline bool operator==(floating_point lhs, floating_point rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(floating_point lhs, floating_point rhs) noexcept { return !(lhs == rhs); }

struct socket_group
{
	bool is_valid() const noexcept
	{
		int sum = r + g + b + w;
		return 0 < sum && sum <= 6;
	}

	int r = 0;
	int g = 0;
	int b = 0;
	int w = 0;
};

inline bool operator==(socket_group lhs, socket_group rhs) noexcept
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.w) == std::tie(rhs.r, rhs.g, rhs.b, rhs.w);
}
inline bool operator!=(socket_group lhs, socket_group rhs) noexcept { return !(lhs == rhs); }

enum class influence { shaper, elder, crusader, redeemer, hunter, warlord };
enum class rarity { normal, magic, rare, unique };
enum class shape { circle, diamond, hexagon, square, star, triangle };
// suit is named "suit" to avoid name conflict with "color"
enum class suit { red, green, blue, white, brown, yellow };

struct string
{
	std::string value;
};

inline bool operator==(const string& lhs, const string& rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(const string& lhs, const string& rhs) noexcept { return !(lhs == rhs); }

} // namespace fs::lang
