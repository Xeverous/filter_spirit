#pragma once

namespace fs::lang {

struct influence_info
{
	bool is_none() const noexcept
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

}
