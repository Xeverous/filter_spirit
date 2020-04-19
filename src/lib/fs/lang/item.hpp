#pragma once

#include <fs/lang/primitive_types.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <string>
#include <optional>
#include <numeric>
#include <vector>

namespace fs::lang
{

template <typename T, std::size_t N>
using svector = boost::container::static_vector<T, N>;

enum class item_validity
{
	valid,

	resonator_or_abyss_socket_linked,
	more_than_6_sockets,
	negative_item_level,
	negative_drop_level,
	negative_height,
	negative_width,
	negative_quality,
	negative_stack_size,
	negative_gem_level,
	negative_map_tier,
	empty_class,
	empty_base_type,
	unidentified_with_explicit_mods
};

enum class socket_color { r, g, b, w, a, d }; // a = abyss, d = resonator (ctrl+C reports resonators with D sockets)

struct linked_sockets
{
	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		// resonator and abyss sockets can not be linked with others
		const auto it = std::find(sockets.begin(), sockets.end(),
			[](socket_color c){ return c == socket_color::a || c == socket_color::d; });

		if (it != sockets.end() && sockets.size() > 1u)
			return item_validity::resonator_or_abyss_socket_linked;

		return item_validity::valid;
	}

	svector<socket_color, 6> sockets;
};

struct socket_info
{
	// return largest group of links
	int links() const noexcept
	{
		return std::max(groups.begin(), groups.end(), [](linked_sockets s){ return s.sockets.size(); });
	}

	// return total number of sockets
	int sockets() const noexcept
	{
		return std::accumulate(groups.begin(), groups.end(), 0, [](linked_sockets s){ return s.sockets.size(); });
	}

	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		// an item can have at most 6 sockets
		if (sockets() > 6)
			return item_validity::more_than_6_sockets;

		for (linked_sockets group : groups)
			if (auto status = group.is_valid(); status != item_validity::valid)
				return status;

		return item_validity::valid;
	}

	svector<linked_sockets, 6> groups;
};

struct influence_info
{
	bool shaper   = false;
	bool elder    = false;
	bool crusader = false;
	bool redeemer = false;
	bool hunter   = false;
	bool warlord  = false;
};

/**
 * @class describes properties of a dropped item
 */
struct item
{
	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		if (height <= 0)
			return item_validity::negative_height;

		if (width <= 0)
			return item_validity::negative_width;

		if (class_.empty())
			return item_validity::empty_class;

		if (base_type.empty())
			return item_validity::empty_base_type;

		// We do not test drop_level vs item_level because some item sources
		// can bypass it (eg. +level strongboxes, divination card exchange).

		// We test quality but given previous breakage with influence (assertion that
		// an item can not be shaper and elder at the same time) it is likely that
		// at some point GGG will introduce negative quality. It could be actually
		// useful for uniques such as the Forbidden Taste.
		if (quality && *quality < 0)
			return item_validity::negative_quality;

		if (socket_info)
			if (auto status = (*socket_info).is_valid(); status != item_validity::valid)
				return status;

		// An item with non-zero visible explicit mods must be identified.
		if (explicit_mods && !(*explicit_mods).empty() && is_identified == false)
			return item_validity::unidentified_with_explicit_mods;

		if (stack_size && *stack_size <= 0)
			return item_validity::negative_stack_size;

		if (gem_level && *gem_level <= 0)
			return item_validity::negative_gem_level;

		if (map_tier && *map_tier <= 0)
			return item_validity::negative_map_tier;

		return item_validity::valid;
	}

	/*
	 * mandatory fields
	 * an item with abnormal values in these will be conidered invalid
	 */
	std::string class_;
	std::string base_type;
	int height = 0;
	int width  = 0;
	bool is_prophecy = false;

	/*
	 * optional fields
	 * Many item classes can not have any of these properties.
	 *
	 * It is important to make a distinction between having any value of the property
	 * and not being able to have it. Item filter blocks can reject an item because
	 * it is impossible to compare lack of property to any value of it. Example:
	 * "Sockets < 6" will reject divination cards because a card can not ever have
	 * sockets - not being able to have sockets is not treated as 0.
	 *
	 * The value of being able or not is the same for all items of the same class - for
	 * example, all gems can be corrupted, all equipment items can be identified, all
	 * divination cards can not be corrputed, all maps can not be synthesised.
	 *
	 * There seems to be some exceptions to this rule unfortunately, see
	 * https://old.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/fjxf1mx
	 */
	std::optional<int> item_level;
	std::optional<int> drop_level;
	std::optional<int> quality;
	std::optional<rarity_type> rarity_;
	std::optional<socket_info> socket_info;
	std::optional<std::vector<std::string>> explicit_mods;
	std::optional<std::vector<std::string>> enchantments;
	std::optional<int> stack_size;
	std::optional<int> gem_level;
	std::optional<int> map_tier;
	std::optional<bool> is_identified;
	std::optional<bool> is_corrupted;
	std::optional<bool> is_fractured_item;
	std::optional<bool> is_synthesised_item;
	std::optional<bool> is_shaped_map;
	std::optional<bool> is_elder_map;
	std::optional<bool> is_blighted_map;
	std::optional<influence_info> influence;

	// A logic-irrelevant field. Only for the user.
	std::string description;
};

}
