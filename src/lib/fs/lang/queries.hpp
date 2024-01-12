#pragma once

#include <fs/utility/better_enum.hpp>

namespace fs::lang
{
	// TODO remove and replace with Autogen "string"
	BETTER_ENUM(item_category, int,
		currency,
		fragments,
		delirium_orbs,
		cards,
		essences,
		fossils,
		resonators,
		scarabs,
		incubators,
		oils,
		vials,

		gems,

		bases,

		uniques_eq_unambiguous,
		uniques_eq_ambiguous,
		uniques_flasks_unambiguous,
		uniques_flasks_ambiguous,
		uniques_jewels_unambiguous,
		uniques_jewels_ambiguous,
		uniques_maps_unambiguous,
		uniques_maps_ambiguous

		// not supported (yet)
		// invitations
	)

	inline bool is_uniques_category(item_category cat) noexcept
	{
		return
			   cat == +item_category::uniques_eq_unambiguous
			|| cat == +item_category::uniques_eq_ambiguous
			|| cat == +item_category::uniques_flasks_unambiguous
			|| cat == +item_category::uniques_flasks_ambiguous
			|| cat == +item_category::uniques_jewels_unambiguous
			|| cat == +item_category::uniques_jewels_ambiguous
			|| cat == +item_category::uniques_maps_unambiguous
			|| cat == +item_category::uniques_maps_ambiguous;
	}
}
