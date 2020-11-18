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
#include <initializer_list>

namespace fs::lang
{

namespace item_class_names {

	// core
	constexpr auto currency_stackable = "Stackable Currency";
	constexpr auto currency_delve = "Delve Stackable Socketable Currency";
	constexpr auto incubator = "Incubator";
	constexpr auto harvest_seed = "Harvest Seed";
	constexpr auto harvest_seed_enhancer = "Seed Enhancer";
	constexpr auto gem_active = "Active Skill Gems";
	constexpr auto gem_support = "Support Skill Gems";
	constexpr auto piece = "Piece";
	constexpr auto incursion_item = "Incursion Item";
	constexpr auto atlas_item = "Atlas Region Upgrade Item";
	constexpr auto labyrinth_trinket = "Labyrinth Trinket";
	constexpr auto labyrinth_item = "Labyrinth Item";
	constexpr auto metamorph_sample = "Metamorph Sample";
	constexpr auto divination_card = "Divination Card";
	constexpr auto leaguestone = "Leaguestones";
	constexpr auto map = "Maps";
	constexpr auto map_fragment = "Map Fragments";
	constexpr auto misc_map_item = "Misc Map Item";
	constexpr auto quest_item = "Quest Items";
	constexpr auto pantheon_sould = "Pantheon Soul";

	// equipment - main parts
	constexpr auto eq_gloves = "Gloves";
	constexpr auto eq_boots = "Boots";
	constexpr auto eq_helmet = "Helmets";
	constexpr auto eq_body = "Body Armours";

	// equipment - 1h
	constexpr auto eq_claw = "Claws";
	constexpr auto eq_dagger = "Daggers";
	constexpr auto eq_rune_dagger = "Rune Daggers";
	constexpr auto eq_sword_1h = "One Hand Swords";
	constexpr auto eq_sword_thrusting = "Thrusting One Hand Swords";
	constexpr auto eq_axe_1h = "One Hand Axes";
	constexpr auto eq_mace_1h = "One Hand Maces";
	constexpr auto eq_sceptre = "Sceptres";
	constexpr auto eq_wand = "Wands";

	// equipment - 2h
	constexpr auto eq_bow = "Bows";
	constexpr auto eq_staff = "Staves";
	constexpr auto eq_warstaff = "Warstaves";
	constexpr auto eq_sword_2h = "Two Hand Swords";
	constexpr auto eq_axe_2h = "Two Hand Axes";
	constexpr auto eq_mace_2h = "Two Hand Maces";
	constexpr auto eq_fishing_rod = "Fishing Rods";

	// equipment - offhand
	constexpr auto eq_quiver = "Quivers";
	constexpr auto eq_shield = "Shields";

	// equipment - jewellery
	constexpr auto eq_amulet = "Amulets";
	constexpr auto eq_ring = "Rings";
	constexpr auto eq_belt = "Belts";

	// jewels
	constexpr auto jewel = "Jewel";
	constexpr auto jewel_abyss = "Abyss Jewel";

	// flasks
	constexpr auto flask_life = "Life Flasks";
	constexpr auto flask_mana = "Mana Flasks";
	constexpr auto flask_hybrid = "Hybrid Flasks";
	constexpr auto flask_utility = "Utility Flasks";
	constexpr auto flask_utility_critical = "Critical Utility Flasks";

}

template <typename T, std::size_t N>
using svector = boost::container::static_vector<T, N>;

enum class item_validity
{
	valid,

	identified_without_name,
	unidentified_with_name,
	unidentified_with_explicit_mods,
	normal_rarity_with_name,
	nonnormal_rarity_without_name,
	non_unique_replica,
	resonator_or_abyss_socket_linked,
	more_than_6_sockets,
	invalid_item_level,
	invalid_drop_level,
	invalid_height,
	invalid_width,
	invalid_quality,
	invalid_stack_size,
	invalid_gem_level,
	invalid_map_tier,
	empty_class,
	empty_base_type
};

enum class socket_color { r, g, b, w, a, d }; // a = abyss, d = resonator (ctrl+C reports resonators with D sockets)

// represents a single group of linked sockets
struct linked_sockets
{
	linked_sockets() = default;

	linked_sockets(std::initializer_list<socket_color> colors)
	{
		for (socket_color c : colors)
			sockets.push_back(c);
	}

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
		if (name.has_value() && !is_identified)
			return item_validity::unidentified_with_name;

		if (!name.has_value() && is_identified)
			return item_validity::identified_without_name;

		if (name.has_value() && rarity_ == rarity_type::normal)
			return item_validity::normal_rarity_with_name;

		if (!name.has_value() && rarity_ != rarity_type::normal)
			return item_validity::nonnormal_rarity_without_name;

		if (height <= 0)
			return item_validity::invalid_height;

		if (width <= 0)
			return item_validity::invalid_width;

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
			return item_validity::invalid_quality;

		if (auto status = sockets.verify(); status != item_validity::valid)
			return status;

		// An item with non-zero visible explicit mods must be identified.
		if (!explicit_mods.empty() && is_identified == false)
			return item_validity::unidentified_with_explicit_mods;

		if (stack_size <= 0 || stack_size > max_stack_size)
			return item_validity::invalid_stack_size;

		if (gem_level <= 0) // do not test gem_level > max_gem_level because corruption can bypass it
			return item_validity::invalid_gem_level;

		if (map_tier <= 0)
			return item_validity::invalid_map_tier;

		if (rarity_ != rarity_type::unique && is_replica)
			return item_validity::non_unique_replica;

		return item_validity::valid;
	}

	/*
	 * mandatory fields
	 * an item with abnormal values in these will be conidered invalid
	 */
	std::string class_;
	std::optional<std::string> name; // only for IDed items with non-normal rarity
	std::string base_type;
	// these 2 should be overwritten whenever an item is created - size 0x0 makes no sense
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
	 * for disscussion about it. Since then, every item has some default sentinel values (usually 0)
	 * from item filter's point of view.
	 *
	 * Sentinel values below are values which filter considers when an item has no such properties.
	 */
	static constexpr auto sentinel_item_level = 0;
	static constexpr auto sentinel_drop_level = 1;
	static constexpr auto sentinel_quality    = 0;
	static constexpr auto sentinel_rarity     = rarity_type::normal;
	static constexpr auto sentinel_stack_size = 1;
	static constexpr auto sentinel_gem_level  = 0;
	static constexpr auto sentinel_map_tier   = 0;

	int item_level = sentinel_item_level;
	int drop_level = sentinel_drop_level;
	int quality = sentinel_quality;
	rarity_type rarity_ = sentinel_rarity;
	socket_info sockets = {};
	std::vector<std::string> explicit_mods = {};
	std::vector<std::string> enchantments_labirynth = {};
	std::vector<std::string> enchantments_passive_nodes = {};
	std::vector<std::string> annointments = {};
	int corrupted_mods = 0;
	int stack_size = sentinel_stack_size;
	int max_stack_size = sentinel_stack_size; // filters do not support this property
	int gem_level = sentinel_gem_level;
	int max_gem_level = sentinel_gem_level; // filters do not support this property
	int map_tier = sentinel_map_tier;
	bool is_identified = false;
	bool is_corrupted = false;
	bool is_mirrored = false;
	bool is_fractured_item = false;
	bool is_synthesised_item = false;
	bool is_shaped_map = false;
	bool is_elder_map = false;
	bool is_blighted_map = false;
	bool is_replica = false;
	bool is_alternate_quality = false;
	influence_info influence = {};

	// A logic-irrelevant field. Only for the user.
	std::string description;
};

int snprintf_dropped_item_label(char* buf, std::size_t buf_size, const item& itm);

}
