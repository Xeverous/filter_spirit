#pragma once

#include <fs/lang/limits.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/keywords.hpp>

#include <tuple>
#include <string>
#include <optional>
#include <variant>
#include <string_view>

namespace fs::lang
{

/*
 * Some types have defaulted origin to make braced-init
 * possible with less arguments than the number of members.
 *
 * Other members are left without any "= init" to force
 * correct initialization in code by issuing compiler warnings.
 */

struct none
{
	position_tag origin;
};

inline bool operator==(none, none) noexcept { return true; }
inline bool operator!=(none, none) noexcept { return false; }

struct temp
{
	position_tag origin;
};

inline bool operator==(temp, temp) noexcept { return true; }
inline bool operator!=(temp, temp) noexcept { return false; }

struct boolean
{
	bool value;
	position_tag origin = {};
};

inline std::string_view to_string_view(bool value) noexcept
{
	return value ? lang::keywords::rf::true_ : lang::keywords::rf::false_;
}

inline bool operator==(boolean lhs, boolean rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(boolean lhs, boolean rhs) noexcept { return !(lhs == rhs); }

struct integer
{
	int value;
	position_tag origin = {};
};

inline bool operator==(integer lhs, integer rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(integer lhs, integer rhs) noexcept { return !(lhs == rhs); }

struct fractional
{
	double value;
	position_tag origin = {};
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
	position_tag origin = {};
};

inline bool operator==(socket_spec lhs, socket_spec rhs) noexcept
{
	return std::tie(lhs.r, lhs.g, lhs.b, lhs.w) == std::tie(rhs.r, rhs.g, rhs.b, rhs.w);
}
inline bool operator!=(socket_spec lhs, socket_spec rhs) noexcept { return !(lhs == rhs); }

enum class influence_type { shaper, elder, crusader, redeemer, hunter, warlord };
struct influence
{
	influence_type value;
	position_tag origin = {};
};

inline bool operator==(influence lhs, influence rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(influence lhs, influence rhs) noexcept { return !(lhs == rhs); }

enum class rarity_type { normal, magic, rare, unique };
struct rarity
{
	rarity_type value;
	position_tag origin = {};
};

inline std::string_view to_string_view(rarity_type rarity_)
{
	namespace kw = keywords::rf;

	if (rarity_ == rarity_type::normal)
		return kw::normal;
	else if (rarity_ == rarity_type::magic)
		return kw::magic;
	else if (rarity_ == rarity_type::rare)
		return kw::rare;
	else if (rarity_ == rarity_type::unique)
		return kw::unique;
	else
		return "?";
}

inline bool operator==(rarity lhs, rarity rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(rarity lhs, rarity rhs) noexcept { return !(lhs == rhs); }
inline bool operator< (rarity lhs, rarity rhs) noexcept { return lhs.value <  rhs.value; }
inline bool operator> (rarity lhs, rarity rhs) noexcept { return lhs.value >  rhs.value; }
inline bool operator<=(rarity lhs, rarity rhs) noexcept { return lhs.value <= rhs.value; }
inline bool operator>=(rarity lhs, rarity rhs) noexcept { return lhs.value >= rhs.value; }

enum class shape_type { circle, diamond, hexagon, square, star, triangle, cross, moon, raindrop, kite, pentagon, upside_down_house };
struct shape
{
	shape_type value;
	position_tag origin = {};
};

inline bool operator==(shape lhs, shape rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(shape lhs, shape rhs) noexcept { return !(lhs == rhs); }

// suit_type is named "suit_type" to avoid name conflict with "color"
enum class suit_type { red, green, blue, white, brown, yellow, cyan, grey, orange, pink, purple };
struct suit
{
	suit_type value;
	position_tag origin = {};
};

inline bool operator==(suit lhs, suit rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(suit lhs, suit rhs) noexcept { return !(lhs == rhs); }

enum class shaper_voice_line_type { mirror, exalted, divine, general, regal, chaos, fusing, alchemy, vaal, blessed };
struct shaper_voice_line
{
	shaper_voice_line_type value;
	position_tag origin = {};
};

inline bool operator==(shaper_voice_line lhs, shaper_voice_line rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(shaper_voice_line lhs, shaper_voice_line rhs) noexcept { return !(lhs == rhs); }

struct string
{
	std::string value;
	position_tag origin = {};
};

inline bool operator==(const string& lhs, const string& rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator!=(const string& lhs, const string& rhs) noexcept { return !(lhs == rhs); }

// values are implied by keywords {Enable|Disable}DropSound{IfAlertSound|}
struct switch_drop_sound
{
	bool enable;
	bool if_alert_sound;
};

inline bool operator==(switch_drop_sound lhs, switch_drop_sound rhs) noexcept
{
	return lhs.enable == rhs.enable && lhs.if_alert_sound == rhs.if_alert_sound;
}
inline bool operator!=(switch_drop_sound lhs, switch_drop_sound rhs) noexcept { return !(lhs == rhs); }

} // namespace fs::lang
