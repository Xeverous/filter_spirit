#pragma once

#include <fs/utility/better_enum.hpp>

namespace fs::lang
{
	// TODO rename Autogen names to be easier to understand
	BETTER_ENUM(autogen_category, int,
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

	inline bool is_uniques_category(autogen_category cat) noexcept
	{
		return
			   cat == +autogen_category::uniques_eq_unambiguous
			|| cat == +autogen_category::uniques_eq_ambiguous
			|| cat == +autogen_category::uniques_flasks_unambiguous
			|| cat == +autogen_category::uniques_flasks_ambiguous
			|| cat == +autogen_category::uniques_jewels_unambiguous
			|| cat == +autogen_category::uniques_jewels_ambiguous
			|| cat == +autogen_category::uniques_maps_unambiguous
			|| cat == +autogen_category::uniques_maps_ambiguous;
	}
}
