#pragma once

#include <fs/lang/limits.hpp>

#include <tuple>
#include <string>
#include <optional>
#include <variant>

namespace fs::lang
{

struct none {};

inline bool operator==(none, none) noexcept { return true; }
inline bool operator!=(none, none) noexcept { return false; }

struct temp {};

inline bool operator==(temp, temp) noexcept { return true; }
inline bool operator!=(temp, temp) noexcept { return false; }

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

struct fractional
{
	explicit fractional(double value) : value(value) {}
	explicit fractional(integer n) : value(n.value) {}

	double value;
};

inline bool operator==(fractional lhs, fractional rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(fractional lhs, fractional rhs) noexcept { return !(lhs == rhs); }

struct socket_spec
{
	bool is_num_valid() const noexcept
	{
		if (!num)
			return true;

		return limits::min_item_sockets <= *num && *num <= limits::max_item_sockets;
	}

	bool is_valid() const noexcept
	{
		auto sum = r + g + b + w + a + d;
		return limits::min_item_sockets <= sum && sum <= limits::max_item_sockets && is_num_valid();
	}

	std::optional<int> num = std::nullopt;
	int r = 0;
	int g = 0;
	int b = 0;
	int w = 0; // white sockets
	int a = 0; // abyss sockets
	int d = 0; // delve sockets (on resonators)
};

inline bool operator==(socket_spec lhs, socket_spec rhs) noexcept
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.w) == std::tie(rhs.r, rhs.g, rhs.b, rhs.w);
}
inline bool operator!=(socket_spec lhs, socket_spec rhs) noexcept { return !(lhs == rhs); }

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
