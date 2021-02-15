#pragma once

namespace fs::lang
{
	enum class item_category
	{
		currency,
		fragments,
		delirium_orbs,
		cards,
		prophecies,
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
		// enchants_helmet
		// invitations
	};

	inline bool is_uniques_category(item_category cat) noexcept
	{
		return
			   cat == item_category::uniques_eq_unambiguous
			|| cat == item_category::uniques_eq_ambiguous
			|| cat == item_category::uniques_flasks_unambiguous
			|| cat == item_category::uniques_flasks_ambiguous
			|| cat == item_category::uniques_jewels_unambiguous
			|| cat == item_category::uniques_jewels_ambiguous
			|| cat == item_category::uniques_maps_unambiguous
			|| cat == item_category::uniques_maps_ambiguous;
	}
}
