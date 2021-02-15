#pragma once

namespace fs::lang {

struct influence_info
{
	constexpr bool is_none() const noexcept
	{
		return !(shaper || elder || crusader || redeemer || hunter || warlord);
	}

	bool shaper   = false;
	bool elder    = false;
	bool crusader = false;
	bool redeemer = false;
	bool hunter   = false;
	bool warlord  = false;
};

constexpr bool operator==(influence_info lhs, influence_info rhs) noexcept
{
	return
		   lhs.shaper   == rhs.shaper
		&& lhs.elder    == rhs.elder
		&& lhs.crusader == rhs.crusader
		&& lhs.redeemer == rhs.redeemer
		&& lhs.hunter   == rhs.hunter
		&& lhs.warlord  == rhs.warlord;
}

constexpr bool operator!=(influence_info lhs, influence_info rhs) noexcept
{
	return !(lhs == rhs);
}

}
