#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/influence_info.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <string>
#include <optional>
#include <numeric>
#include <vector>
#include <string_view>
#include <string>

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

// represents a single group of linked sockets
struct linked_sockets
{
	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		// resonator and abyss sockets can not be linked with others
		const auto it = std::find_if(sockets.begin(), sockets.end(),
			[](socket_color c){ return c == socket_color::a || c == socket_color::d; });

		if (it != sockets.end() && sockets.size() > 1u)
			return item_validity::resonator_or_abyss_socket_linked;

		return item_validity::valid;
	}

	int count_of(socket_color color) const
	{
		return std::count(sockets.begin(), sockets.end(), color);
	}

	int count_red()   const { return count_of(socket_color::r); }
	int count_green() const { return count_of(socket_color::g); }
	int count_blue()  const { return count_of(socket_color::b); }
	int count_white() const { return count_of(socket_color::w); }
	// there is no count A and D because these should never be linked together

	svector<socket_color, 6> sockets;
};

// represents item sockets state
struct socket_info
{
	// return largest group of links
	int links() const noexcept
	{
		int result = 0;

		for (const auto& group : groups)
			result = std::max<int>(result, group.sockets.size());

		return result;
	}

	// return total number of sockets
	int sockets() const noexcept
	{
		return std::accumulate(groups.begin(), groups.end(), 0,
			[](int sum, const linked_sockets& s){ return sum + s.sockets.size(); });
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
			if (auto status = group.verify(); status != item_validity::valid)
				return status;

		return item_validity::valid;
	}

	// any item has at most 6 distinct socket groups
	svector<linked_sockets, 6> groups;
};

[[nodiscard]] std::optional<socket_info> to_socket_info(std::string_view str);

[[nodiscard]] char to_char(socket_color color);
[[nodiscard]] std::string to_string(socket_info info);

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
		if (quality < 0)
			return item_validity::negative_quality;

		if (auto status = sockets.verify(); status != item_validity::valid)
			return status;

		// An item with non-zero visible explicit mods must be identified.
		if (!explicit_mods.empty() && is_identified == false)
			return item_validity::unidentified_with_explicit_mods;

		if (stack_size <= 0)
			return item_validity::negative_stack_size;

		if (gem_level <= 0)
			return item_validity::negative_gem_level;

		if (map_tier <= 0)
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
	bool is_prophecy = false; // if true, base type has an additional meaning (Prophecy condition)

	/*
	 * "seemingly optional" fields
	 * Many item classes do not seem to have any of these properties.
	 *
	 * In the past, it was important to make a distinction between having any value of the
	 * property and not being able to have it. Item filter blocks could reject an item
	 * because it was impossible to compare lack of property to any value of it. Example:
	 * "Sockets < 6" rejected divination cards because a card could not ever have sockets.
	 *
	 * This was all unintentional, see
	 * https://old.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/fjxf1mx
	 * for disscussion about it. Since then, every item has some default values (usually 0)
	 * from item filter's point of view.
	 *
	 * Default values below are values which filter considers when an item has no such properties.
	 */
	int item_level = 0;
	int drop_level = 1;
	int quality = 0;
	rarity_type rarity_ = rarity_type::normal;
	socket_info sockets = {};
	std::vector<std::string> explicit_mods = {};
	std::vector<std::string> enchantments_labirynth = {};
	std::vector<std::string> enchantments_passive_nodes = {};
	std::vector<std::string> annointments = {};
	int corrupted_mods = 0;
	int stack_size = 1;
	int gem_level = 0;
	int map_tier = 0;
	bool is_identified = false;
	bool is_corrupted = false;
	bool is_mirrored = false;
	bool is_fractured_item = false;
	bool is_synthesised_item = false;
	bool is_shaped_map = false;
	bool is_elder_map = false;
	bool is_blighted_map = false;
	influence_info influence = {};

	// A logic-irrelevant field. Only for the user.
	std::string description;
};

}
