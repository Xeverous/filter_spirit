#pragma once

#include <fs/utility/string_helpers.hpp>

#include <boost/container/static_vector.hpp>

#include <string>
#include <initializer_list>
#include <optional>
#include <unordered_map>
#include <vector>

/*
 * This implementation is somewhat a duplicate of item_database.
 * It is used for Autogen's StackSize (in case some stackable item drops in stacks).
 * item_database will probably be rewritten on new GUI and then both can be merged.
 * Rigth now knowledge of stack sizes is hardcoded until a way to parse external data is found.
 *
 * 2 core features are required:
 * - Autogen: ability to inform the caller what's the stack size of a particular item.
 * - GUI: use knowledge of names, DropLevel and other properties for filter simulation.
 */

namespace fs::lang::loot {

struct known_item_properties
{
	known_item_properties(int max_stack_size, std::optional<int> drop_level = std::nullopt, int width = 1, int height = 1)
	: max_stack_size(max_stack_size), drop_level(drop_level), width(width), height(height)
	{}

	static known_item_properties legacy(
		int max_stack_size, std::optional<int> drop_level = std::nullopt, int width = 1, int height = 1)
	{
		known_item_properties props(max_stack_size, drop_level, width, height);
		props.drop_disabled = true;
		return props;
	}

	int max_stack_size; // for unstackable items, just use 1
	std::optional<int> drop_level;
	int width;
	int height;
	bool drop_disabled = false;
};

struct known_item
{
	std::string name;
	known_item_properties properties;
};

// unused code so far, might be useful later for loot simulation
class known_item_collection
{
public:
	static constexpr auto max_groups = 4;
	using group_type = std::pair<const known_item*, const known_item*>;
	using container_type = boost::container::static_vector<group_type, max_groups>;

	known_item_collection() = default;

	known_item_collection(std::initializer_list<group_type> groups)
	: m_groups(groups) {}

	class iterator
	{
	public:
		iterator(container_type::const_iterator it)
		: m_groups_it(it) {}

		friend bool operator==(iterator lhs, iterator rhs)
		{
			if (lhs.m_groups_it == rhs.m_groups_it)
				return lhs.m_ingroup_idx == rhs.m_ingroup_idx;

			return false;
		}

		friend bool operator!=(iterator lhs, iterator rhs)
		{
			return !(lhs == rhs);
		}

		iterator& operator++()
		{
			if (++m_ingroup_idx == current_group_size()) {
				++m_groups_it;
				m_ingroup_idx = 0;
			}

			return *this;
		}

		iterator operator++(int)
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		const known_item& operator*() const
		{
			return (m_groups_it->first)[m_ingroup_idx];
		}

		const known_item* operator->() const
		{
			return m_groups_it->first + m_ingroup_idx;
		}

	private:
		container_type::size_type current_group_size() const
		{
			// silence convertion warning, result here is always non-negative
			return static_cast<container_type::size_type>(m_groups_it->second - m_groups_it->first);
		}

		container_type::const_iterator m_groups_it;
		container_type::size_type m_ingroup_idx = 0;
	};

	iterator begin() const
	{
		return {m_groups.begin()};
	}

	iterator end() const
	{
		return {m_groups.end()};
	}

	std::optional<int> max_stack_size_of(std::string_view item_name) const
	{
		for (const known_item& itm : *this) {
			if (itm.name == item_name)
				return itm.properties.max_stack_size;
		}

		return std::nullopt;
	}

private:
	container_type m_groups;
};

class known_items_store
{
public:
	// returns nullptr when not found
	[[nodiscard]] const known_item_properties* find_item(const std::string& name) const
	{
		const auto it = m_data.find(name);
		if (it != m_data.end())
			return &it->second;

		for (const known_item& item_family : m_item_families) {
			if (utility::contains(name, item_family.name))
				return &item_family.properties;
		}

		return nullptr;
	}

	// convenience overload
	[[nodiscard]] std::optional<int> max_stack_size_of(const std::string& name) const
	{
		const known_item_properties* properties = find_item(name);
		if (properties == nullptr)
			return std::nullopt;

		return properties->max_stack_size;
	}

	template <typename... Containers>
	static known_items_store create(std::initializer_list<known_item> item_families, const Containers&... containers)
	{
		known_items_store result;

		([&result](const auto& container) {
			for (const known_item& item : container)
				result.m_data.insert({item.name, item.properties});
		}(containers), ...);

		for (const known_item& item : item_families)
			result.m_item_families.push_back(item);

		return result;
	}

private:
	/*
	 * The items could be stored in a much more organized way, however:
	 * - poe.ninja has a specific, sometimes unique categorization (e.g. per game mechanic)
	 * - in-game market and website trade can have different categorization
	 * - filters use classes for filtering, which does not always match the above
	 * - loot generation/simulation can be much more complex than all points above
	 *
	 * Thus, a plain hash table is used. Immune to categorization problems and still very efficient.
	 */
	std::unordered_map<std::string, known_item_properties> m_data;
	// for more-often changed items with same stack size (e.g. Tattoos, Scarabs)
	std::vector<known_item> m_item_families;
};

namespace poe1 {

// lists sorted by game version where particular item family was introduced

inline const std::initializer_list<known_item> currency_basic = {
	// (by drop level, then alphabetical)
	{"Ancient Orb",                   known_item_properties(   20,  1)},
	{"Armourer's Scrap",              known_item_properties(   40,  1)},
	{"Blacksmith's Whetstone",        known_item_properties(   20,  1)},
	{"Engineer's Orb",                known_item_properties(   20,  1)},
	{"Fracturing Orb",                known_item_properties(   20,  1)},
	{"Harbinger's Orb",               known_item_properties(   20,  1)},
	{"Orb of Augmentation",           known_item_properties(   30,  1)},
	{"Orb of Transmutation",          known_item_properties(   40,  1)},
	{"Scroll of Wisdom",              known_item_properties(   40,  1)},
	{"Chromatic Orb",                 known_item_properties(   20,  2)},
	{"Glassblower's Bauble",          known_item_properties(   20,  2)},
	{"Orb of Alchemy",                known_item_properties(   20,  2)},
	{"Orb of Alteration",             known_item_properties(   20,  2)},
	{"Orb of Chance",                 known_item_properties(   20,  2)},
	{"Orb of Regret",                 known_item_properties(   40,  2)},
	{"Portal Scroll",                 known_item_properties(   40,  2)},
	{"Jeweller's Orb",                known_item_properties(   20,  8)},
	{"Orb of Fusing",                 known_item_properties(   20,  8)},
	{"Chaos Orb",                     known_item_properties(   20, 12)},
	{"Gemcutter's Prism",             known_item_properties(   20, 12)},
	{"Orb of Scouring",               known_item_properties(   30, 12)},
	{"Regal Orb",                     known_item_properties(   20, 12)},
	{"Vaal Orb",                      known_item_properties(   20, 12)},
	{"Orb of Binding",                known_item_properties(   20, 25)},
	{"Blessed Orb",                   known_item_properties(   20, 35)},
	{"Divine Orb",                    known_item_properties(   20, 35)},
	{"Exalted Orb",                   known_item_properties(   20, 35)},
	{"Mirror of Kalandra",            known_item_properties(   10, 35)},
	{"Orb of Annulment",              known_item_properties(   20, 35)},
	{"Sacred Orb",                    known_item_properties(   10, 35)},
	{"Enkindling Orb",                known_item_properties(   20, 40)},
	{"Hinekora's Lock",               known_item_properties(   10, 40)},
	{"Instilling Orb",                known_item_properties(   20, 40)},
	// officially 40 but in reality drops only from T16+ - TODO DropLevel needs testing in-game
	{"Reflecting Mist",               known_item_properties(   10, 40)},
	{"Rogue's Marker",                known_item_properties(50000, 48)},
	{"Stacked Deck",                  known_item_properties(   20, 50)},
	{"Cartographer's Chisel",         known_item_properties(   20, 52)},
	// Lesser Eldritch are here - they can drop on T1 maps without Altars (wiki also puts them here)
	{"Lesser Eldritch Ember",         known_item_properties(   10, 68)},
	{"Lesser Eldritch Ichor",         known_item_properties(   10, 68)},
	{"Orb of Horizons",               known_item_properties(   20, 68)},
	{"Volatile Vaal Orb",             known_item_properties(   20, 68)}, // 3.28
	{"Orb of Unmaking",               known_item_properties(   40, 70)},
};

inline const std::initializer_list<known_item> currency_shards = {
	// (by drop level, then alphabetical)
	{"Alchemy Shard",       known_item_properties(20,  1)},
	{"Alteration Shard",    known_item_properties(20,  1)},
	{"Scroll Fragment",     known_item_properties( 5,  1)},
	{"Transmutation Shard", known_item_properties(20,  1)},
	{"Chaos Shard",         known_item_properties(20,  8)},
	{"Engineer's Shard",    known_item_properties(20,  8)},
	{"Regal Shard",         known_item_properties(20, 12)},
	{"Binding Shard",       known_item_properties(20, 25)},
	{"Ancient Shard",       known_item_properties(20, 35)},
	{"Annulment Shard",     known_item_properties(20, 35)},
	{"Exalted Shard",       known_item_properties(20, 35)},
	{"Fracturing Shard",    known_item_properties(20, 35)},
	{"Mirror Shard",        known_item_properties(20, 35)},
	{"Harbinger's Shard",   known_item_properties(20, 52)},
	{"Horizon Shard",       known_item_properties(20, 52)},
};

inline const std::initializer_list<known_item> currency_atlas = {
	// (by drop level, then alphabetical)
	{"Awakener's Orb",                  known_item_properties(10,  1)}, // drops from Sirus
	{"Crusader's Exalted Orb",          known_item_properties(20,  1)}, // drops from Baran
	{"Elder's Exalted Orb",             known_item_properties(20,  1)}, // drops from ((Uber) Uber) Edler
	{"Hunter's Exalted Orb",            known_item_properties(20,  1)}, // drops from Al-Hezmin

	// drops from any Maven's Inviation (any Crucible) or witnessed map bosses
	{"Maven's Chisel of Avarice",       known_item_properties(20,  1)},
	{"Maven's Chisel of Divination",    known_item_properties(20,  1)},
	{"Maven's Chisel of Procurement",   known_item_properties(20,  1)},
	{"Maven's Chisel of Proliferation", known_item_properties(20,  1)},
	{"Maven's Chisel of Scarabs",       known_item_properties(20,  1)},

	{"Orb of Conflict",                 known_item_properties(10,  1)}, // drops from Maven and Invitations
	{"Orb of Dominance",                known_item_properties(10,  1)}, // Elder, Uber Elder, Shaper, Sirus

	// Incarnation bosses
	{"Orb of Intention",                known_item_properties(10,  1)},
	{"Orb of Remembrance",              known_item_properties(10,  1)},
	{"Orb of Unravelling",              known_item_properties(10,  1)},

	{"Redeemer's Exalted Orb",          known_item_properties(20,  1)}, // drops from Veritania
	{"Shaper's Exalted Orb",            known_item_properties(20,  1)}, // drops from Shaper
	{"Warlord's Exalted Orb",           known_item_properties(20,  1)}, // drops from Drox

	// Exarch/Eater monsters or their mini-bosses
	{"Greater Eldritch Ember",          known_item_properties(10, 73)},
	{"Greater Eldritch Ichor",          known_item_properties(10, 73)},
	{"Eldritch Chaos Orb",              known_item_properties(20, 78)},
	{"Eldritch Exalted Orb",            known_item_properties(20, 78)},
	{"Eldritch Orb of Annulment",       known_item_properties(20, 78)},
	{"Grand Eldritch Ember",            known_item_properties(10, 78)},
	{"Grand Eldritch Ichor",            known_item_properties(10, 78)},

	// Exarch/Eater mini-bosses
	{"Exceptional Eldritch Ember",      known_item_properties(10, 81)},
	{"Exceptional Eldritch Ichor",      known_item_properties(10, 81)},

	// drops from any Maven's Inviation (any Crucible)
	{"Crescent Splinter",               known_item_properties(10, 83)},

	// drops in T11+ and in Delve
	{"Valdo's Puzzle Box",              known_item_properties(10, 83)},

	{"Templar Astrolabe",               known_item_properties(10,  1)},
	{"Chaotic Astrolabe",               known_item_properties(10,  1)},
	{"Deceptive Astrolabe",             known_item_properties(10,  1)},
	{"Fruiting Astrolabe",              known_item_properties(10,  1)},
	{"Fungal Astrolabe",                known_item_properties(10,  1)},
	{"Grasping Astrolabe",              known_item_properties(10,  1)},
	{"Lightless Astrolabe",             known_item_properties(10,  1)},
	{"Nameless Astrolabe",              known_item_properties(10,  1)},
	{"Runic Astrolabe",                 known_item_properties(10,  1)},
	{"Timeless Astrolabe",              known_item_properties(10,  1)},
};

inline const std::initializer_list<known_item> currency_legacy = {
	{"Infused Engineer's Orb", known_item_properties::legacy(20, 1)}, // 3.11 - 3.19 (legacy now)
	{"Veiled Scarab",          known_item_properties::legacy(20, 1)}, // 3.24+ (converted legacy items)
};

// note: drop-level for Essences has weak meaning as corruption
// and atlas passives can produce better tiers than original
inline const std::initializer_list<known_item> currency_essences = { // 2.4
	// Group A
	{"Whispering Essence of Greed",    known_item_properties(9,  1)},
	{"Muttering Essence of Greed",     known_item_properties(9, 12)},
	{"Weeping Essence of Greed",       known_item_properties(9, 30)},
	{"Wailing Essence of Greed",       known_item_properties(9, 48)},
	{"Screaming Essence of Greed",     known_item_properties(9, 68)},
	{"Shrieking Essence of Greed",     known_item_properties(9, 73)},
	{"Deafening Essence of Greed",     known_item_properties(9, 73)},

	{"Whispering Essence of Contempt", known_item_properties(9,  1)},
	{"Muttering Essence of Contempt",  known_item_properties(9, 12)},
	{"Weeping Essence of Contempt",    known_item_properties(9, 30)},
	{"Wailing Essence of Contempt",    known_item_properties(9, 48)},
	{"Screaming Essence of Contempt",  known_item_properties(9, 68)},
	{"Shrieking Essence of Contempt",  known_item_properties(9, 73)},
	{"Deafening Essence of Contempt",  known_item_properties(9, 73)},

	{"Whispering Essence of Hatred",   known_item_properties(9,  1)},
	{"Muttering Essence of Hatred",    known_item_properties(9, 12)},
	{"Weeping Essence of Hatred",      known_item_properties(9, 30)},
	{"Wailing Essence of Hatred",      known_item_properties(9, 48)},
	{"Screaming Essence of Hatred",    known_item_properties(9, 68)},
	{"Shrieking Essence of Hatred",    known_item_properties(9, 73)},
	{"Deafening Essence of Hatred",    known_item_properties(9, 73)},

	{"Whispering Essence of Woe",      known_item_properties(9,  1)},
	{"Muttering Essence of Woe",       known_item_properties(9, 12)},
	{"Weeping Essence of Woe",         known_item_properties(9, 30)},
	{"Wailing Essence of Woe",         known_item_properties(9, 48)},
	{"Screaming Essence of Woe",       known_item_properties(9, 68)},
	{"Shrieking Essence of Woe",       known_item_properties(9, 73)},
	{"Deafening Essence of Woe",       known_item_properties(9, 73)},

	// Group B
	{"Muttering Essence of Fear",      known_item_properties(9, 12)},
	{"Weeping Essence of Fear",        known_item_properties(9, 30)},
	{"Wailing Essence of Fear",        known_item_properties(9, 48)},
	{"Screaming Essence of Fear",      known_item_properties(9, 68)},
	{"Shrieking Essence of Fear",      known_item_properties(9, 73)},
	{"Deafening Essence of Fear",      known_item_properties(9, 73)},

	{"Muttering Essence of Anger",     known_item_properties(9, 12)},
	{"Weeping Essence of Anger",       known_item_properties(9, 30)},
	{"Wailing Essence of Anger",       known_item_properties(9, 48)},
	{"Screaming Essence of Anger",     known_item_properties(9, 68)},
	{"Shrieking Essence of Anger",     known_item_properties(9, 73)},
	{"Deafening Essence of Anger",     known_item_properties(9, 73)},

	{"Muttering Essence of Torment",   known_item_properties(9, 12)},
	{"Weeping Essence of Torment",     known_item_properties(9, 30)},
	{"Wailing Essence of Torment",     known_item_properties(9, 48)},
	{"Screaming Essence of Torment",   known_item_properties(9, 68)},
	{"Shrieking Essence of Torment",   known_item_properties(9, 73)},
	{"Deafening Essence of Torment",   known_item_properties(9, 73)},

	{"Muttering Essence of Sorrow",    known_item_properties(9, 12)},
	{"Weeping Essence of Sorrow",      known_item_properties(9, 30)},
	{"Wailing Essence of Sorrow",      known_item_properties(9, 48)},
	{"Screaming Essence of Sorrow",    known_item_properties(9, 68)},
	{"Shrieking Essence of Sorrow",    known_item_properties(9, 73)},
	{"Deafening Essence of Sorrow",    known_item_properties(9, 73)},

	// Group C
	{"Weeping Essence of Rage",        known_item_properties(9, 30)},
	{"Wailing Essence of Rage",        known_item_properties(9, 48)},
	{"Screaming Essence of Rage",      known_item_properties(9, 68)},
	{"Shrieking Essence of Rage",      known_item_properties(9, 73)},
	{"Deafening Essence of Rage",      known_item_properties(9, 73)},

	{"Weeping Essence of Suffering",   known_item_properties(9, 30)},
	{"Wailing Essence of Suffering",   known_item_properties(9, 48)},
	{"Screaming Essence of Suffering", known_item_properties(9, 68)},
	{"Shrieking Essence of Suffering", known_item_properties(9, 73)},
	{"Deafening Essence of Suffering", known_item_properties(9, 73)},

	{"Weeping Essence of Wrath",       known_item_properties(9, 30)},
	{"Wailing Essence of Wrath",       known_item_properties(9, 48)},
	{"Screaming Essence of Wrath",     known_item_properties(9, 68)},
	{"Shrieking Essence of Wrath",     known_item_properties(9, 73)},
	{"Deafening Essence of Wrath",     known_item_properties(9, 73)},

	{"Weeping Essence of Doubt",       known_item_properties(9, 30)},
	{"Wailing Essence of Doubt",       known_item_properties(9, 48)},
	{"Screaming Essence of Doubt",     known_item_properties(9, 68)},
	{"Shrieking Essence of Doubt",     known_item_properties(9, 73)},
	{"Deafening Essence of Doubt",     known_item_properties(9, 73)},

	// Group D
	{"Wailing Essence of Loathing",    known_item_properties(9, 48)},
	{"Screaming Essence of Loathing",  known_item_properties(9, 68)},
	{"Shrieking Essence of Loathing",  known_item_properties(9, 73)},
	{"Deafening Essence of Loathing",  known_item_properties(9, 73)},

	{"Wailing Essence of Zeal",        known_item_properties(9, 48)},
	{"Screaming Essence of Zeal",      known_item_properties(9, 68)},
	{"Shrieking Essence of Zeal",      known_item_properties(9, 73)},
	{"Deafening Essence of Zeal",      known_item_properties(9, 73)},

	{"Wailing Essence of Anguish",     known_item_properties(9, 48)},
	{"Screaming Essence of Anguish",   known_item_properties(9, 68)},
	{"Shrieking Essence of Anguish",   known_item_properties(9, 73)},
	{"Deafening Essence of Anguish",   known_item_properties(9, 73)},

	{"Wailing Essence of Spite",       known_item_properties(9, 48)},
	{"Screaming Essence of Spite",     known_item_properties(9, 68)},
	{"Shrieking Essence of Spite",     known_item_properties(9, 73)},
	{"Deafening Essence of Spite",     known_item_properties(9, 73)},

	// Group E
	{"Screaming Essence of Scorn",     known_item_properties(9, 68)},
	{"Shrieking Essence of Scorn",     known_item_properties(9, 73)},
	{"Deafening Essence of Scorn",     known_item_properties(9, 73)},

	{"Screaming Essence of Envy",      known_item_properties(9, 68)},
	{"Shrieking Essence of Envy",      known_item_properties(9, 73)},
	{"Deafening Essence of Envy",      known_item_properties(9, 73)},

	{"Screaming Essence of Misery",    known_item_properties(9, 68)},
	{"Shrieking Essence of Misery",    known_item_properties(9, 73)},
	{"Deafening Essence of Misery",    known_item_properties(9, 73)},

	{"Screaming Essence of Dread",     known_item_properties(9, 68)},
	{"Shrieking Essence of Dread",     known_item_properties(9, 73)},
	{"Deafening Essence of Dread",     known_item_properties(9, 73)},

	// Group F (corruption only)
	{"Essence of Insanity",            known_item_properties(9, 73)},
	{"Essence of Horror",              known_item_properties(9, 73)},
	{"Essence of Delirium",            known_item_properties(9, 73)},
	{"Essence of Hysteria",            known_item_properties(9, 73)},

	// other
	{"Remnant of Corruption",          known_item_properties(9, 1)}
};

inline const std::initializer_list<known_item> currency_breach_splinters = { // 2.5
	{"Splinter of Xoph",      known_item_properties(100, 1)},
	{"Splinter of Tul",       known_item_properties(100, 1)},
	{"Splinter of Esh",       known_item_properties(100, 1)},
	{"Splinter of Uul-Netol", known_item_properties(100, 1)},
	{"Splinter of Chayula",   known_item_properties(100, 1)}
};

inline const std::initializer_list<known_item> currency_breach_blessings = { // 2.5
	{"Blessing of Xoph",      known_item_properties(10, 1)},
	{"Blessing of Tul",       known_item_properties(10, 1)},
	{"Blessing of Esh",       known_item_properties(10, 1)},
	{"Blessing of Uul-Netol", known_item_properties(10, 1)},
	{"Blessing of Chayula",   known_item_properties(10, 1)},
	// 3.28 replaces all, all blessings are now legacy items
	{"Flesh of Xesht",        known_item_properties(10, 1)},
};

inline const std::initializer_list<known_item> currency_incursion_vials = { // 3.3
	{"Vial of Awakening",     known_item_properties(10, 50)},
	{"Vial of Consequence",   known_item_properties(10, 50)},
	{"Vial of Dominance",     known_item_properties(10, 50)},
	{"Vial of Fate",          known_item_properties(10, 50)},
	{"Vial of Summoning",     known_item_properties(10, 50)},
	{"Vial of the Ritual",    known_item_properties(10, 50)},
	{"Vial of Transcendence", known_item_properties(10, 50)},
	{"Vial of Sacrifice",     known_item_properties(10, 68)},
	{"Vial of the Ghost",     known_item_properties(10, 68)}
};

inline const std::initializer_list<known_item> currency_delve_fossils = { // 3.4
	{"Aberrant Fossil",     known_item_properties(20, 1)},
	{"Aetheric Fossil",     known_item_properties(20, 1)},
	{"Bloodstained Fossil", known_item_properties(10, 1)},
	{"Bound Fossil",        known_item_properties(20, 1)},
	{"Corroded Fossil",     known_item_properties(20, 1)},
	{"Deft Fossil",         known_item_properties(20, 1)},
	{"Dense Fossil",        known_item_properties(20, 1)},
	{"Faceted Fossil",      known_item_properties(10, 1)},
	{"Fractured Fossil",    known_item_properties(10, 1)},
	{"Frigid Fossil",       known_item_properties(20, 1)},
	{"Fundamental Fossil",  known_item_properties(20, 1)},
	{"Gilded Fossil",       known_item_properties(10, 1)},
	{"Glyphic Fossil",      known_item_properties(10, 1)},
	{"Hollow Fossil",       known_item_properties(10, 1)},
	{"Jagged Fossil",       known_item_properties(20, 1)},
	{"Lucent Fossil",       known_item_properties(20, 1)},
	{"Metallic Fossil",     known_item_properties(20, 1)},
	{"Opulent Fossil",      known_item_properties(20, 1)},
	{"Prismatic Fossil",    known_item_properties(20, 1)},
	{"Pristine Fossil",     known_item_properties(20, 1)},
	{"Sanctified Fossil",   known_item_properties(10, 1)},
	{"Scorched Fossil",     known_item_properties(20, 1)},
	{"Serrated Fossil",     known_item_properties(20, 1)},
	{"Shuddering Fossil",   known_item_properties(20, 1)},
	{"Tangled Fossil",      known_item_properties(10, 1)}
};

inline const std::initializer_list<known_item> currency_delve_resonators = { // 3.4
	{"Primitive Chaotic Resonator", known_item_properties(10, 34, 1, 1)},
	{"Potent Chaotic Resonator",    known_item_properties(10, 34, 1, 2)},
	{"Powerful Chaotic Resonator",  known_item_properties(10, 34, 2, 2)},
	{"Prime Chaotic Resonator",     known_item_properties(10, 68, 2, 2)}
};

inline const std::initializer_list<known_item> currency_delve_resonators_legacy = { // 3.4 - 3.16
	{"Primitive Alchemical Resonator", known_item_properties::legacy(10,  1, 1, 1)},
	{"Potent Alchemical Resonator",    known_item_properties::legacy(10,  1, 1, 2)},
	{"Powerful Alchemical Resonator",  known_item_properties::legacy(10, 34, 2, 2)},
	{"Prime Alchemical Resonator",     known_item_properties::legacy(10, 68, 2, 2)}
};

inline const std::initializer_list<known_item> currency_delve_resonators_ruthless = currency_delve_resonators_legacy;

inline const std::initializer_list<known_item> currency_legion_splinters = { // 3.7
	{"Timeless Vaal Splinter",           known_item_properties(100, 1)},
	{"Timeless Karui Splinter",          known_item_properties(100, 1)},
	{"Timeless Eternal Empire Splinter", known_item_properties(100, 1)},
	{"Timeless Templar Splinter",        known_item_properties(100, 1)},
	{"Timeless Maraketh Splinter",       known_item_properties(100, 1)}
};

inline const std::initializer_list<known_item> currency_blight_legacy = { // 3.17 - 3.25 (legacy now)
	{"Oil Extractor", known_item_properties::legacy(10)}
};

inline const std::initializer_list<known_item> currency_blight_oils_regular = { // 3.8
	{"Clear Oil",      known_item_properties(10,  1)},
	{"Sepia Oil",      known_item_properties(10, 10)},
	{"Amber Oil",      known_item_properties(10, 19)},
	{"Verdant Oil",    known_item_properties(10, 27)},
	{"Teal Oil",       known_item_properties(10, 36)},
	{"Azure Oil",      known_item_properties(10, 44)},
	{"Indigo Oil",     known_item_properties(10, 48)},
	{"Violet Oil",     known_item_properties(10, 52)},
	{"Crimson Oil",    known_item_properties(10, 60)},
	{"Black Oil",      known_item_properties(10, 68)},
	{"Opalescent Oil", known_item_properties(10, 73)},
	{"Silver Oil",     known_item_properties(10, 78)},
	{"Golden Oil",     known_item_properties(10, 80)}
};

inline const std::initializer_list<known_item> currency_blight_oils_special = { // 3.8
	{"Reflective Oil", known_item_properties(10,  1)},
	{"Tainted Oil",    known_item_properties(10,  1)},
	{"Prismatic Oil",  known_item_properties(10, 80)}
};

inline const std::initializer_list<known_item> currency_catalysts = { // 3.9
	{"Abrasive Catalyst",     known_item_properties(20,  1)},
	{"Accelerating Catalyst", known_item_properties(20,  1)},
	{"Imbued Catalyst",       known_item_properties(20,  1)},
	{"Intrinsic Catalyst",    known_item_properties(20,  1)},
	{"Noxious Catalyst",      known_item_properties(20,  1)},
	{"Tainted Catalyst",      known_item_properties(20,  1)},
	{"Turbulent Catalyst",    known_item_properties(20,  1)},
	{"Unstable Catalyst",     known_item_properties(20,  1)},
	{"Fertile Catalyst",      known_item_properties(20, 68)},
	{"Prismatic Catalyst",    known_item_properties(20, 68)},
	{"Tempering Catalyst",    known_item_properties(20, 68)},
	{"Sinistral Catalyst",    known_item_properties(20,  1)}, // 3.28
	{"Dextral Catalyst",      known_item_properties(20,  1)}, // 3.28
};

inline const std::initializer_list<known_item> currency_delirium_generic = { // 3.10
	{"Simulacrum Splinter", known_item_properties(300,  1)},
	{"Refracting Fog",      known_item_properties( 20, 68)}, // 3.28
};

inline const std::initializer_list<known_item> currency_delirium_orbs = { // 3.10
	{"Abyssal Delirium Orb",        known_item_properties(10)},
	{"Armoursmith's Delirium Orb",  known_item_properties(10)},
	{"Blacksmith's Delirium Orb",   known_item_properties(10)},
	{"Blighted Delirium Orb",       known_item_properties(10)},
	{"Cartographer's Delirium Orb", known_item_properties(10)},
	{"Diviner's Delirium Orb",      known_item_properties(10)},
	{"Fine Delirium Orb",           known_item_properties(10)},
	{"Foreboding Delirium Orb",     known_item_properties(10)},
	{"Fossilised Delirium Orb",     known_item_properties(10)},
	{"Fragmented Delirium Orb",     known_item_properties(10)},
	{"Jeweller's Delirium Orb",     known_item_properties(10)},
	{"Obscured Delirium Orb",       known_item_properties(10)},
	{"Singular Delirium Orb",       known_item_properties(10)},
	{"Skittering Delirium Orb",     known_item_properties(10)},
	{"Thaumaturge's Delirium Orb",  known_item_properties(10)},
	{"Timeless Delirium Orb",       known_item_properties(10)},
	{"Whispering Delirium Orb",     known_item_properties(10)},
};

inline const std::initializer_list<known_item> currency_delirium_orbs_legacy = { // 3.10 - ?
	{"Challenging Delirium Orb", known_item_properties::legacy(10)},
	{"Kalguuran Delirium Orb",   known_item_properties::legacy(10)},
	{"Imperial Delirium Orb",    known_item_properties::legacy(10)},
	{"Primal Delirium Orb",      known_item_properties::legacy(10)}
};

// In Ruthless only 1 generic Delirum Orb is available
inline const std::initializer_list<known_item> currency_delirium_orbs_ruthless = {
	{"Delirium Orb", known_item_properties(10)}
};

inline const std::initializer_list<known_item> currency_harbinger_scrolls = { // 3.11
	{"Deregulation Scroll",    known_item_properties(10, 1)},
	{"Electroshock Scroll",    known_item_properties(10, 1)},
	{"Fragmentation Scroll",   known_item_properties(10, 1)},
	{"Haemocombustion Scroll", known_item_properties(10, 1)},
	{"Specularity Scroll",     known_item_properties(10, 1)},
	{"Time-light Scroll",      known_item_properties(10, 1)},
};

inline const std::initializer_list<known_item> currency_heist = { // 3.12
	{"Tempering Orb", known_item_properties(20, 1)},
	{"Tailoring Orb", known_item_properties(20, 1)},
};

inline const std::initializer_list<known_item> currency_ritual = { // 3.13
	{"Ritual Splinter", known_item_properties(100, 1)},
	{"Ritual Vessel",   known_item_properties( 10, 1)}
};

// Not obtainable in Ruthless
inline const std::initializer_list<known_item> currency_betrayal = { // 3.14 (though changed extensively)
	{"Veiled Chaos Orb",   known_item_properties(20, 61)},
	{"Veiled Exalted Orb", known_item_properties(20, 61)},
};

inline const std::initializer_list<known_item> currency_expedition = { // 3.15
	{"Astragali",        known_item_properties(1000, 1)},
	{"Scrap Metal",      known_item_properties(1000, 1)},
	{"Exotic Coinage",   known_item_properties(1000, 1)},
	{"Burial Medallion", known_item_properties(1000, 1)}
};

inline const std::initializer_list<known_item> currency_beyond = { // 3.16
	{"Tainted Armourer's Scrap",       known_item_properties(40,  1)},
	{"Tainted Blacksmith's Whetstone", known_item_properties(20,  1)},
	{"Tainted Chromatic Orb",          known_item_properties(20,  2)},
	{"Tainted Mythic Orb",             known_item_properties(20,  2)},
	{"Tainted Jeweller's Orb",         known_item_properties(20,  8)},
	{"Tainted Chaos Orb",              known_item_properties(20, 12)},
	{"Tainted Exalted Orb",            known_item_properties(20, 35)},
	{"Tainted Orb of Fusing",          known_item_properties(20, 50)},
	{"Tainted Divine Teardrop",        known_item_properties(10, 68)},
};

inline const std::initializer_list<known_item> currency_scouting_reports = { // 3.17
	{"Blighted Scouting Report",      known_item_properties(20, 69)},
	{"Comprehensive Scouting Report", known_item_properties(20, 69)},
	{"Delirious Scouting Report",     known_item_properties(20, 69)},
	{"Explorer's Scouting Report",    known_item_properties(20, 69)},
	{"Operative's Scouting Report",   known_item_properties(20, 69)},
	{"Singular Scouting Report",      known_item_properties(20, 69)},
	{"Vaal Scouting Report",          known_item_properties(20, 69)},
	{"Influenced Scouting Report",    known_item_properties(20, 81)},
	{"Otherworldly Scouting Report",  known_item_properties(20, 81)},
};

inline const std::initializer_list<known_item> currency_harvest_lifeforce_regular = { // 3.19
	{"Primal Crystallised Lifeforce", known_item_properties(50000)},
	{"Vivid Crystallised Lifeforce",  known_item_properties(50000)},
	{"Wild Crystallised Lifeforce",   known_item_properties(50000)}
};

inline const std::initializer_list<known_item> currency_harvest_lifeforce_special = { // 3.19
	{"Sacred Crystallised Lifeforce", known_item_properties(10)},
	{"Crystallised Rancour",          known_item_properties(10)}, // 3.28
};

inline const std::initializer_list<known_item> currency_runegrafts = { // 3.26
	{"Runegraft of Bellows",       known_item_properties(10)},
	{"Runegraft of Blasphemy",     known_item_properties(10)},
	{"Runegraft of Gemcraft",      known_item_properties(10)},
	{"Runegraft of Loyalty",       known_item_properties(10)},
	{"Runegraft of Quaffing",      known_item_properties(10)},
	{"Runegraft of Recompense",    known_item_properties(10)},
	{"Runegraft of Restitching",   known_item_properties(10)},
	{"Runegraft of Stability",     known_item_properties(10)},
	{"Runegraft of the Angler",    known_item_properties(10)},
	{"Runegraft of the Bound",     known_item_properties(10)},
	{"Runegraft of the Combatant", known_item_properties(10)},
	{"Runegraft of the Fortress",  known_item_properties(10)},
	{"Runegraft of the Jeweller",  known_item_properties(10)},
	{"Runegraft of the Novamark",  known_item_properties(10)},
	{"Runegraft of the River",     known_item_properties(10)},
	{"Runegraft of the Sinistral", known_item_properties(10)},
	{"Runegraft of the Soulwick",  known_item_properties(10)},
	{"Runegraft of the Warp",      known_item_properties(10)},
	{"Runegraft of the Witchmark", known_item_properties(10)},
	{"Runegraft of Time",          known_item_properties(10)},
	{"Runegraft of Treachery",     known_item_properties(10)}
};

inline const std::initializer_list<known_item> currency_implant = {
	{"Augmentation Implant", known_item_properties(10)},
	{"Regal Implant",        known_item_properties(10)},
	{"Exalted Implant",      known_item_properties(10)},
	{"Unstable Implant",     known_item_properties(10)}
};

inline const std::initializer_list<known_item> currency_foulborn = {
	{"Foulborn Orb of Augmentation", known_item_properties(10)},
	{"Foulborn Regal Orb",           known_item_properties(10)},
	{"Foulborn Exalted Orb",         known_item_properties(10)}
};

inline const std::initializer_list<known_item> fragments_lures = { // 3.11 - 3.19 (legacy now)
	{"Craicic Lure",  known_item_properties::legacy(20)},
	{"Farric Lure",   known_item_properties::legacy(20)},
	{"Fenumal Lure",  known_item_properties::legacy(20)},
	{"Saqawine Lure", known_item_properties::legacy(20)}
};

inline const known_item_collection group_currency = {{
	std::make_pair(currency_basic .begin(), currency_basic .end()),
	std::make_pair(currency_shards.begin(), currency_shards.end()),
	std::make_pair(currency_atlas .begin(), currency_atlas .end()),
}};

// UNFINISHED:
// Scarabs (3.5)  (uses family matching)
// Tattoos (3.22) (uses family matching)
//   Omens (3.22) (NOT IMPLEMENTED)
// (more items but they are not currency)

inline const auto known_items = known_items_store::create(
	{
		known_item{"Tattoo", known_item_properties(10)},
		known_item{"Scarab", known_item_properties(20)}
	},
	currency_basic,
	currency_shards,
	currency_atlas,
	currency_legacy,
	currency_essences,
	currency_breach_splinters,
	currency_breach_blessings,
	currency_incursion_vials,
	currency_delve_fossils,
	currency_delve_resonators,
	currency_delve_resonators_legacy,
	// currency_delve_resonators_ruthless, (same as currency_delve_resonators_legacy)
	currency_legion_splinters,
	currency_blight_legacy,
	currency_blight_oils_regular,
	currency_blight_oils_special,
	currency_catalysts,
	currency_delirium_generic,
	currency_delirium_orbs,
	currency_delirium_orbs_legacy,
	currency_delirium_orbs_ruthless,
	currency_harbinger_scrolls,
	currency_heist,
	currency_ritual,
	currency_betrayal,
	currency_expedition,
	currency_beyond,
	currency_scouting_reports,
	currency_harvest_lifeforce_regular,
	currency_harvest_lifeforce_special,
	currency_runegrafts,
	currency_implant,
	currency_foulborn,
	fragments_lures
);

} // namespace poe1

namespace poe2 {

// (by drop level, then alphabetical)

inline const std::initializer_list<known_item> currency_basic = {
	{"Orb of Augmentation", known_item_properties(30, 1)},
	{"Orb of Transmutation", known_item_properties(40, 1)},
	{"Scroll of Wisdom", known_item_properties(40, 1)},
	{"Arcanist's Etcher", known_item_properties(20, 5)},
	{"Armourer's Scrap", known_item_properties(20, 5)},
	{"Artificer's Orb", known_item_properties(20, 5)},
	{"Blacksmith's Whetstone", known_item_properties(20, 5)},
	{"Exalted Orb", known_item_properties(20, 5)},
	{"Orb of Alchemy", known_item_properties(20, 5)},
	{"Regal Orb", known_item_properties(20, 5)},
	{"Chaos Orb", known_item_properties(20, 12)},
	{"Gemcutter's Prism", known_item_properties(20, 12)},
	{"Glassblower's Bauble", known_item_properties(20, 12)},
	{"Orb of Chance", known_item_properties(10, 12)},
	{"Lesser Jeweller's Orb", known_item_properties(20, 25)},
	{"Divine Orb", known_item_properties(10, 35)},
	{"Mirror of Kalandra", known_item_properties(10, 35)},
	{"Orb of Annulment", known_item_properties(20, 35)},
	{"Vaal Orb", known_item_properties(20, 35)},
	{"Greater Jeweller's Orb", known_item_properties(20, 45)},
	{"Hinekora's Lock", known_item_properties(10, 46)},
	{"Perfect Jeweller's Orb", known_item_properties(20, 65)},
	// Greater currency
	{"Greater Orb of Augmentation", known_item_properties(30, 57)},
	{"Greater Orb of Transmutation", known_item_properties(40, 57)},
	{"Greater Chaos Orb", known_item_properties(20, 72)},
	{"Greater Exalted Orb", known_item_properties(20, 72)},
	{"Greater Regal Orb", known_item_properties(20, 72)},
	// Perfect currency
	{"Perfect Orb of Augmentation", known_item_properties(30, 72)},
	{"Perfect Orb of Transmutation", known_item_properties(40, 72)},
	{"Perfect Chaos Orb", known_item_properties(20, 79)},
	{"Perfect Exalted Orb", known_item_properties(20, 79)},
	{"Perfect Regal Orb", known_item_properties(20, 79)},
	// special drops
	{"Fracturing Orb", known_item_properties(20, 1)}, // (only from Cleansed monsters)
	{"Albino Rhoa Feather", known_item_properties(10, 1)},
	{"Cryptic Key", known_item_properties(1, 1)}, // (Jado exclusive)
	// splinters
	{"Breach Splinter",     known_item_properties(300, 1)},
	{"Petition Splinter",   known_item_properties(300, 1)}, // 0.3.0, legacy no use item since 0.5.0
	{"Runic Splinter",      known_item_properties(300, 1)}, // 0.3.0, legacy no use item since 0.5.0
	{"Simulacrum Splinter", known_item_properties(300, 1)},
};

inline const std::initializer_list<known_item> currency_shards = {
	{"Chance Shard", known_item_properties(10, 1)},
	{"Transmutation Shard", known_item_properties(10, 1)},
	{"Artificer's Shard", known_item_properties(10, 5)},
	{"Regal Shard", known_item_properties(10, 5)}
};

inline const std::initializer_list<known_item> currency_essences = {
	{"Essence of Alacrity", known_item_properties(10, 1)},
	{"Essence of Command", known_item_properties(10, 1)},
	{"Essence of Delirium", known_item_properties(10, 1)},
	{"Essence of Grounding", known_item_properties(10, 1)},
	{"Essence of Horror", known_item_properties(10, 1)},
	{"Essence of Hysteria", known_item_properties(10, 1)},
	{"Essence of Insanity", known_item_properties(10, 1)},
	{"Essence of Insulation", known_item_properties(10, 1)},
	{"Essence of Opulence", known_item_properties(10, 1)},
	{"Essence of Thawing", known_item_properties(10, 1)},
	{"Essence of the Abyss", known_item_properties(10, 1)},
	{"Greater Essence of Alacrity", known_item_properties(10, 1)},
	{"Greater Essence of Command", known_item_properties(10, 1)},
	{"Greater Essence of Grounding", known_item_properties(10, 1)},
	{"Greater Essence of Insulation", known_item_properties(10, 1)},
	{"Greater Essence of Opulence", known_item_properties(10, 1)},
	{"Greater Essence of Thawing", known_item_properties(10, 1)},
	{"Lesser Essence of Alacrity", known_item_properties(10, 1)},
	{"Lesser Essence of Command", known_item_properties(10, 1)},
	{"Lesser Essence of Grounding", known_item_properties(10, 1)},
	{"Lesser Essence of Insulation", known_item_properties(10, 1)},
	{"Lesser Essence of Opulence", known_item_properties(10, 1)},
	{"Lesser Essence of Thawing", known_item_properties(10, 1)},
	{"Perfect Essence of Alacrity", known_item_properties(10, 1)},
	{"Perfect Essence of Command", known_item_properties(10, 1)},
	{"Perfect Essence of Grounding", known_item_properties(10, 1)},
	{"Perfect Essence of Insulation", known_item_properties(10, 1)},
	{"Perfect Essence of Opulence", known_item_properties(10, 1)},
	{"Perfect Essence of Thawing", known_item_properties(10, 1)},
	{"Lesser Essence of Battle", known_item_properties(10, 20)},
	{"Lesser Essence of Enhancement", known_item_properties(10, 20)},
	{"Lesser Essence of Seeking", known_item_properties(10, 20)},
	{"Lesser Essence of Sorcery", known_item_properties(10, 20)},
	{"Lesser Essence of the Infinite", known_item_properties(10, 20)},
	{"Lesser Essence of Abrasion", known_item_properties(10, 25)},
	{"Lesser Essence of Electricity", known_item_properties(10, 25)},
	{"Lesser Essence of Flames", known_item_properties(10, 25)},
	{"Lesser Essence of Haste", known_item_properties(10, 25)},
	{"Lesser Essence of Ice", known_item_properties(10, 25)},
	{"Lesser Essence of Ruin", known_item_properties(10, 25)},
	{"Lesser Essence of the Body", known_item_properties(10, 25)},
	{"Lesser Essence of the Mind", known_item_properties(10, 25)},
	{"Essence of Abrasion", known_item_properties(10, 40)},
	{"Essence of Battle", known_item_properties(10, 40)},
	{"Essence of Electricity", known_item_properties(10, 40)},
	{"Essence of Enhancement", known_item_properties(10, 40)},
	{"Essence of Flames", known_item_properties(10, 40)},
	{"Essence of Haste", known_item_properties(10, 40)},
	{"Essence of Ice", known_item_properties(10, 40)},
	{"Essence of Ruin", known_item_properties(10, 40)},
	{"Essence of Seeking", known_item_properties(10, 40)},
	{"Essence of Sorcery", known_item_properties(10, 40)},
	{"Essence of the Body", known_item_properties(10, 40)},
	{"Essence of the Infinite", known_item_properties(10, 40)},
	{"Essence of the Mind", known_item_properties(10, 40)},
	{"Greater Essence of Abrasion", known_item_properties(10, 60)},
	{"Greater Essence of Battle", known_item_properties(10, 60)},
	{"Greater Essence of Electricity", known_item_properties(10, 60)},
	{"Greater Essence of Enhancement", known_item_properties(10, 60)},
	{"Greater Essence of Flames", known_item_properties(10, 60)},
	{"Greater Essence of Haste", known_item_properties(10, 60)},
	{"Greater Essence of Ice", known_item_properties(10, 60)},
	{"Greater Essence of Ruin", known_item_properties(10, 60)},
	{"Greater Essence of Seeking", known_item_properties(10, 60)},
	{"Greater Essence of Sorcery", known_item_properties(10, 60)},
	{"Greater Essence of the Body", known_item_properties(10, 60)},
	{"Greater Essence of the Infinite", known_item_properties(10, 60)},
	{"Greater Essence of the Mind", known_item_properties(10, 60)},
	{"Perfect Essence of Abrasion", known_item_properties(10, 72)},
	{"Perfect Essence of Battle", known_item_properties(10, 72)},
	{"Perfect Essence of Electricity", known_item_properties(10, 72)},
	{"Perfect Essence of Enhancement", known_item_properties(10, 72)},
	{"Perfect Essence of Flames", known_item_properties(10, 72)},
	{"Perfect Essence of Haste", known_item_properties(10, 72)},
	{"Perfect Essence of Ice", known_item_properties(10, 72)},
	{"Perfect Essence of Ruin", known_item_properties(10, 72)},
	{"Perfect Essence of Seeking", known_item_properties(10, 72)},
	{"Perfect Essence of Sorcery", known_item_properties(10, 72)},
	{"Perfect Essence of the Body", known_item_properties(10, 72)},
	{"Perfect Essence of the Infinite", known_item_properties(10, 72)},
	{"Perfect Essence of the Mind", known_item_properties(10, 72)},
	{"Essence of the Breach", known_item_properties(10, 1)}, // 0.5.0
};

inline const std::initializer_list<known_item> currency_abyss = {
	// Crafting
	{"Gnawed Collarbone", known_item_properties(20, 25)},
	{"Gnawed Jawbone", known_item_properties(20, 25)},
	{"Gnawed Rib", known_item_properties(20, 25)},
	{"Preserved Collarbone", known_item_properties(20, 61)},
	{"Preserved Jawbone", known_item_properties(20, 61)},
	{"Preserved Rib", known_item_properties(20, 61)},
	{"Preserved Cranium", known_item_properties(20, 65)},
	{"Preserved Vertebrae", known_item_properties(20, 65)}, // legacy since 0.4
	{"Ancient Collarbone", known_item_properties(20, 75)},
	{"Ancient Jawbone", known_item_properties(20, 75)},
	{"Ancient Rib", known_item_properties(20, 75)},
	{"Altered Collarbone", known_item_properties(20, 1)}, // 0.5.0, drops from Genesis Tree
};

inline const std::initializer_list<known_item> currency_catalysts = {
	{"Adaptive Catalyst",    known_item_properties(10, 30)},
	{"Carapace Catalyst",    known_item_properties(10, 30)},
	{"Chayula's Catalyst",   known_item_properties(10, 30)},
	{"Esh's Catalyst",       known_item_properties(10, 30)},
	{"Flesh Catalyst",       known_item_properties(10, 30)},
	{"Neural Catalyst",      known_item_properties(10, 30)},
	{"Reaver Catalyst",      known_item_properties(10, 30)},
	{"Sibilant Catalyst",    known_item_properties(10, 30)},
	{"Skittering Catalyst",  known_item_properties(10, 30)},
	{"Tul's Catalyst",       known_item_properties(10, 30)},
	{"Uul-Netol's Catalyst", known_item_properties(10, 30)},
	{"Xoph's Catalyst",      known_item_properties(10, 30)},
	{"Necrotic Catalyst",    known_item_properties(10, 30)}, // 0.5.2
	// 0.5.2
	{"Refined Adaptive Catalyst",    known_item_properties(10, 50)},
	{"Refined Carapace Catalyst",    known_item_properties(10, 50)},
	{"Refined Chayula's Catalyst",   known_item_properties(10, 50)},
	{"Refined Esh's Catalyst",       known_item_properties(10, 50)},
	{"Refined Flesh Catalyst",       known_item_properties(10, 50)},
	{"Refined Neural Catalyst",      known_item_properties(10, 50)},
	{"Refined Reaver Catalyst",      known_item_properties(10, 50)},
	{"Refined Sibilant Catalyst",    known_item_properties(10, 50)},
	{"Refined Skittering Catalyst",  known_item_properties(10, 50)},
	{"Refined Tul's Catalyst",       known_item_properties(10, 50)},
	{"Refined Uul-Netol's Catalyst", known_item_properties(10, 50)},
	{"Refined Xoph's Catalyst",      known_item_properties(10, 50)},
	{"Refined Necrotic Catalyst",    known_item_properties(10, 50)},
};

inline const std::initializer_list<known_item> currency_emotions = {
	{"Diluted Liquid Greed", known_item_properties(10, 1)},
	{"Diluted Liquid Guilt", known_item_properties(10, 1)},
	{"Diluted Liquid Ire", known_item_properties(10, 1)},
	{"Liquid Despair", known_item_properties(10, 1)},
	{"Liquid Disgust", known_item_properties(10, 1)},
	{"Liquid Envy", known_item_properties(10, 1)},
	{"Liquid Paranoia", known_item_properties(10, 1)},
	{"Potent Liquid Melancholy", known_item_properties(10, 65)}, // 0.5.0
	{"Potent Liquid Ferocity", known_item_properties(10, 65)}, // 0.5.0
	{"Potent Liquid Contempt", known_item_properties(10, 65)}, // 0.5.0
	{"Concentrated Liquid Fear", known_item_properties(10, 69)},
	{"Concentrated Liquid Suffering", known_item_properties(10, 73)},
	{"Concentrated Liquid Isolation", known_item_properties(10, 77)},
	// 0.5.0
	{"Ancient Diluted Liquid Greed", known_item_properties(10, 1)},
	{"Ancient Diluted Liquid Guilt", known_item_properties(10, 1)},
	{"Ancient Diluted Liquid Ire", known_item_properties(10, 1)},
	{"Ancient Liquid Despair", known_item_properties(10, 1)},
	{"Ancient Liquid Disgust", known_item_properties(10, 1)},
	{"Ancient Liquid Envy", known_item_properties(10, 1)},
	{"Ancient Liquid Paranoia", known_item_properties(10, 1)},
	{"Ancient Potent Liquid Melancholy", known_item_properties(10, 65)},
	{"Ancient Potent Liquid Ferocity", known_item_properties(10, 65)},
	{"Ancient Potent Liquid Contempt", known_item_properties(10, 65)},
	{"Ancient Concentrated Liquid Fear", known_item_properties(10, 69)},
	{"Ancient Concentrated Liquid Suffering", known_item_properties(10, 73)},
	{"Ancient Concentrated Liquid Isolation", known_item_properties(10, 77)},
};

inline const std::initializer_list<known_item> currency_expedition = {
	// since 0.5.0 legacy items with no use
	{"Black Scythe Artifact", known_item_properties(20, 1)},
	{"Broken Circle Artifact", known_item_properties(20, 1)},
	{"Exotic Coinage", known_item_properties(20, 1)},
	{"Order Artifact", known_item_properties(20, 1)},
	{"Sun Artifact", known_item_properties(20, 1)},
	// 0.5.0
	{"Verisium",             known_item_properties(1000, 13)},
	{"Liquid Verisium",      known_item_properties(1000,  1)}, // 0.5.4
	{"Exceptional Verisium", known_item_properties(  50, 65)},
	{"Medved's Crest of the Circle",  known_item_properties(10, 13)},
	{"Vorana's Crest of the Scythe",  known_item_properties(10, 23)},
	{"Uhtred's Crest of the Chalice", known_item_properties(10, 38)},
	{"Olroth's Crest of the Sun",     known_item_properties(10, 65)},
	{"Shattered Triskelion",  known_item_properties(1, 1, 2, 2)}, // note: "The Triskelion Reforged" is a map fragment
	{"Starlit Ore",           known_item_properties(1, 1, 2, 2)},
	{"Revered Starlit Ore",   known_item_properties(1, 1, 2, 4)},
	{"Venerable Starlit Ore", known_item_properties(1, 1, 2, 3)},
	{"Veridical Starlit Ore", known_item_properties(1, 1, 1, 1)},
	{"Warding Starlit Ore",   known_item_properties(1, 1, 2, 4)},
	// 0.5.0
	{"Crackling Flux", known_item_properties(10, 65)},
	{"Blazing Flux",   known_item_properties(10, 65)},
	{"Void Flux",      known_item_properties(10, 65)},
	{"Chilling Flux",  known_item_properties(10, 65)},
	{"Perfect Flux",   known_item_properties(10, 75)},
	// 0.5.0
	{"Runic Alloy",            known_item_properties(10, 13)},
	{"Adaptive Alloy",         known_item_properties(10, 23)},
	{"Expansive Alloy",        known_item_properties(10, 23)},
	{"Protective Alloy",       known_item_properties(10, 23)},
	{"Cyclonic Alloy",         known_item_properties(10, 45)},
	{"Mystic Alloy",           known_item_properties(10, 45)},
	{"Prismatic Alloy",        known_item_properties(10, 45)},
	{"Swift Alloy",            known_item_properties(10, 45)},
	{"Celestial Alloy",        known_item_properties(10, 65)},
	{"Sovereign Alloy",        known_item_properties(10, 65)},
	{"The Runebinder's Alloy", known_item_properties(10, 65)},
	{"The Runefather's Alloy", known_item_properties(10, 65)},
	{"Transcendent Alloy",     known_item_properties(10, 65)},
	// 0.5.0
	// low-level ones may not drop due to Verisium Remnant restrictions (lowest level found is 8)
	{"Thaumaturgic Flux (Level 1)",  known_item_properties(20,  1)},
	{"Thaumaturgic Flux (Level 2)",  known_item_properties(20,  4)},
	{"Thaumaturgic Flux (Level 3)",  known_item_properties(20,  7)},
	{"Thaumaturgic Flux (Level 4)",  known_item_properties(20, 11)},
	{"Thaumaturgic Flux (Level 5)",  known_item_properties(20, 15)},
	{"Thaumaturgic Flux (Level 6)",  known_item_properties(20, 19)},
	{"Thaumaturgic Flux (Level 7)",  known_item_properties(20, 23)},
	{"Thaumaturgic Flux (Level 8)",  known_item_properties(20, 27)},
	{"Thaumaturgic Flux (Level 9)",  known_item_properties(20, 32)},
	{"Thaumaturgic Flux (Level 10)", known_item_properties(20, 37)},
	{"Thaumaturgic Flux (Level 11)", known_item_properties(20, 42)},
	{"Thaumaturgic Flux (Level 12)", known_item_properties(20, 47)},
	{"Thaumaturgic Flux (Level 13)", known_item_properties(20, 53)},
	{"Thaumaturgic Flux (Level 14)", known_item_properties(20, 59)},
	{"Thaumaturgic Flux (Level 15)", known_item_properties(20, 62)},
	{"Thaumaturgic Flux (Level 16)", known_item_properties(20, 62)},
	{"Thaumaturgic Flux (Level 17)", known_item_properties(20, 62)},
	{"Thaumaturgic Flux (Level 18)", known_item_properties(20, 62)},
	{"Thaumaturgic Flux (Level 19)", known_item_properties(20, 62)},
	{"Thaumaturgic Flux (Level 20)", known_item_properties(20, 62)},
};

inline const std::initializer_list<known_item> currency_vaal = { // 0.4.0
	{"Architect's Orb",         known_item_properties(10, 1)},
	{"Crystallised Corruption", known_item_properties(10, 1)},
	{"Vaal Cultivation Orb",    known_item_properties(10, 1)},
	{"Ancient Infuser",         known_item_properties(10, 1)},
	{"Orb of Extraction",       known_item_properties(10, 1)},
	{"Core Destabiliser",       known_item_properties(10, 1)},
	// 0.5.0
	{"Vaal Catalysing Infuser",   known_item_properties(10, 1)},
	{"Vaal Armourer's Infuser",   known_item_properties(10, 1)},
	{"Vaal Blacksmith's Infuser", known_item_properties(10, 1)},
	{"Vaal Arcanist's Infuser",   known_item_properties(10, 1)},
};

// all have max Drop Level of 31, core drop pool
inline const std::initializer_list<known_item> augments_runes_lesser = {
	{"Lesser Adept Rune",       known_item_properties(10,  5)},
	{"Lesser Desert Rune",      known_item_properties(10,  5)},
	{"Lesser Glacial Rune",     known_item_properties(10,  5)},
	{"Lesser Iron Rune",        known_item_properties(10,  5)},
	{"Lesser Resolve Rune",     known_item_properties(10,  5)},
	{"Lesser Robust Rune",      known_item_properties(10,  5)},
	{"Lesser Storm Rune",       known_item_properties(10,  5)},
	{"Lesser Body Rune",        known_item_properties(10, 11)},
	{"Lesser Mind Rune",        known_item_properties(10, 11)},
	{"Lesser Stone Rune",       known_item_properties(10, 16)},
	{"Lesser Vision Rune",      known_item_properties(10, 16)},
	{"Lesser Inspiration Rune", known_item_properties(10, 21)},
	{"Lesser Rebirth Rune",     known_item_properties(10, 21)},
};

// all have max Drop Level of 52, core drop pool
inline const std::initializer_list<known_item> augments_runes_regular = {
	{"Adept Rune",       known_item_properties(10, 31)},
	{"Desert Rune",      known_item_properties(10, 31)},
	{"Glacial Rune",     known_item_properties(10, 31)},
	{"Iron Rune",        known_item_properties(10, 31)},
	{"Resolve Rune",     known_item_properties(10, 31)},
	{"Robust Rune",      known_item_properties(10, 31)},
	{"Storm Rune",       known_item_properties(10, 31)},
	{"Body Rune",        known_item_properties(10, 37)},
	{"Mind Rune",        known_item_properties(10, 37)},
	{"Stone Rune",       known_item_properties(10, 41)},
	{"Vision Rune",      known_item_properties(10, 41)},
	{"Inspiration Rune", known_item_properties(10, 45)},
	{"Rebirth Rune",     known_item_properties(10, 45)},
};

// core drop pool
inline const std::initializer_list<known_item> augments_runes_greater = {
	{"Greater Adept Rune",       known_item_properties(10, 52)},
	{"Greater Desert Rune",      known_item_properties(10, 52)},
	{"Greater Glacial Rune",     known_item_properties(10, 52)},
	{"Greater Iron Rune",        known_item_properties(10, 52)},
	{"Greater Resolve Rune",     known_item_properties(10, 52)},
	{"Greater Robust Rune",      known_item_properties(10, 52)},
	{"Greater Storm Rune",       known_item_properties(10, 52)},
	{"Greater Body Rune",        known_item_properties(10, 57)},
	{"Greater Mind Rune",        known_item_properties(10, 57)},
	{"Greater Stone Rune",       known_item_properties(10, 59)},
	{"Greater Vision Rune",      known_item_properties(10, 59)},
	{"Greater Inspiration Rune", known_item_properties(10, 62)},
	{"Greater Rebirth Rune",     known_item_properties(10, 62)},
};

// not dropping, only through Masterwork Rune
inline const std::initializer_list<known_item> augments_runes_perfect = {
	{"Perfect Adept Rune",       known_item_properties(10, 65)},
	{"Perfect Desert Rune",      known_item_properties(10, 65)},
	{"Perfect Glacial Rune",     known_item_properties(10, 65)},
	{"Perfect Iron Rune",        known_item_properties(10, 65)},
	{"Perfect Resolve Rune",     known_item_properties(10, 65)},
	{"Perfect Robust Rune",      known_item_properties(10, 65)},
	{"Perfect Storm Rune",       known_item_properties(10, 65)},
	{"Perfect Body Rune",        known_item_properties(10, 65)},
	{"Perfect Mind Rune",        known_item_properties(10, 65)},
	{"Perfect Stone Rune",       known_item_properties(10, 65)},
	{"Perfect Vision Rune",      known_item_properties(10, 65)},
	{"Perfect Inspiration Rune", known_item_properties(10, 65)},
	{"Perfect Rebirth Rune",     known_item_properties(10, 65)},
};

// core drop pool
inline const std::initializer_list<known_item> augments_runes_endgame = {
	{"Countess Seske's Rune of Archery",     known_item_properties(10, 65)},
	{"Courtesan Mannan's Rune of Cruelty",   known_item_properties(10, 65)},
	{"Craiceann's Rune of Recovery",         known_item_properties(10, 65)},
	{"Craiceann's Rune of Warding",          known_item_properties(10, 65)},
	{"Farrul's Rune of Grace",               known_item_properties(10, 65)},
	{"Farrul's Rune of the Chase",           known_item_properties(10, 65)},
	{"Farrul's Rune of the Hunt",            known_item_properties(10, 65)},
	{"Fenumus' Rune of Agony",               known_item_properties(10, 65)},
	{"Fenumus' Rune of Draining",            known_item_properties(10, 65)},
	{"Fenumus' Rune of Spinning",            known_item_properties(10, 65)},
	{"Hedgewitch Assandra's Rune of Wisdom", known_item_properties(10, 65)},
	{"Lady Hestra's Rune of Winter",         known_item_properties(10, 65)},
	{"Saqawal's Rune of Erosion",            known_item_properties(10, 65)},
	{"Saqawal's Rune of Memory",             known_item_properties(10, 65)},
	{"Saqawal's Rune of the Sky",            known_item_properties(10, 65)},
	{"Thane Girt's Rune of Wildness",        known_item_properties(10, 65)},
	{"Thane Grannell's Rune of Mastery",     known_item_properties(10, 65)},
	{"Thane Leld's Rune of Spring",          known_item_properties(10, 65)},
	{"Thane Myrk's Rune of Summer",          known_item_properties(10, 65)},
	{"The Greatwolf's Rune of Claws",        known_item_properties(10, 65)},
	{"The Greatwolf's Rune of Willpower",    known_item_properties(10, 65)},
};

// only from The Ezomyte Megaliths
inline const std::initializer_list<known_item> augments_runes_ezomyte_megaliths = {
	{"Greater Rune of Alacrity",   known_item_properties(10, 65)},
	{"Greater Rune of Leadership", known_item_properties(10, 65)},
	{"Greater Rune of Nobility",   known_item_properties(10, 65)},
	{"Greater Rune of Tithing",    known_item_properties(10, 65)},
};

// only from Verisium Remnant encounters
inline const std::initializer_list<known_item> augments_runes_ward = {
	{"Lesser Ward Rune",               known_item_properties(10,  5)},
	{"Lesser Charging Rune",           known_item_properties(10,  5)},
	{"Warding Rune of Reinforcement",  known_item_properties(10, 15)},
	{"Warding Rune of Nourishment",    known_item_properties(10, 15)},
	{"Warding Rune of Protection",     known_item_properties(10, 15)},
	{"Warding Rune of Disintegration", known_item_properties(10, 15)},
	{"Warding Rune of Desperation",    known_item_properties(10, 15)},
	{"Warding Rune of Courage",        known_item_properties(10, 15)},
	{"Warding Rune of Stability",      known_item_properties(10, 30)},
	{"Warding Rune of Glancing",       known_item_properties(10, 30)},
	{"Warding Rune of Heart",          known_item_properties(10, 30)},
	{"Warding Rune of Annihilation",   known_item_properties(10, 30)},
	{"Warding Rune of Salvaging",      known_item_properties(10, 30)},
	{"Warding Rune of Symbiosis",      known_item_properties(10, 30)},
	{"Ward Rune",                      known_item_properties(10, 31)},
	{"Charging Rune",                  known_item_properties(10, 31)},
	{"Warding Rune of Armature",       known_item_properties(10, 45)},
	{"Warding Rune of Obsession",      known_item_properties(10, 45)},
	{"Warding Rune of Equinox",        known_item_properties(10, 45)},
	{"Warding Rune of Bodyguards",     known_item_properties(10, 45)},
	{"Warding Rune of Hollowing",      known_item_properties(10, 45)},
	{"Greater Ward Rune",              known_item_properties(10, 52)},
	{"Greater Charging Rune",          known_item_properties(10, 52)},
};

// only from Verisium Remnant encounters
inline const std::initializer_list<known_item> augments_runes_ancient = {
	{"Ancient Rune of Animosity",   known_item_properties(10, 12)},
	{"Ancient Rune of Control",     known_item_properties(10, 12)},
	{"Ancient Rune of Decay",       known_item_properties(10, 12)},
	{"Ancient Rune of Detonation",  known_item_properties(10, 12)},
	{"Ancient Rune of Discovery",   known_item_properties(10, 12)},
	{"Ancient Rune of Dueling",     known_item_properties(10, 12)},
	{"Ancient Rune of Prowess",     known_item_properties(10, 12)},
	{"Ancient Rune of Retaliation", known_item_properties(10, 12)},
	{"Ancient Rune of Shattering",  known_item_properties(10, 12)},
	{"Ancient Rune of Splinters",   known_item_properties(10, 12)},
	{"Ancient Rune of the Horde",   known_item_properties(10, 12)},
	{"Ancient Rune of the Titan",   known_item_properties(10, 12)},
	{"Ancient Rune of Witchcraft",  known_item_properties(10, 12)},
	{"Rune of Accumulation",        known_item_properties(10, 15)},
	{"Rune of Acrobatics",          known_item_properties(10, 15)},
	{"Rune of Confrontation",       known_item_properties(10, 15)},
	{"Rune of Consistency",         known_item_properties(10, 15)},
	{"Rune of Culmination",         known_item_properties(10, 15)},
	{"Rune of Foundations",         known_item_properties(10, 15)},
	{"Rune of Reach",               known_item_properties(10, 15)},
	{"Rune of Renown",              known_item_properties(10, 15)},
	{"Rune of the Blossom",         known_item_properties(10, 15)},
	{"Rune of the Hunt",            known_item_properties(10, 15)},
	{"Rune of the Prism",           known_item_properties(10, 15)},
	{"Rune of Vital Flame",         known_item_properties(10, 15)},
	{"Rune of Vitality",            known_item_properties(10, 15)},
};

// only from Verisium Remnant encounters
inline const std::initializer_list<known_item> augments_runes_metacraft = {
	{"Masterwork Rune",     known_item_properties(10, 37)},
	{"Betrayal of Aldur",   known_item_properties(10, 65)},
	{"Breath of Aldur",     known_item_properties(10, 65)},
	{"Passion of Aldur",    known_item_properties(10, 65)},
	{"Ire of Aldur",        known_item_properties(10, 65)},
	{"Astrid's Creativity", known_item_properties(10, 65)},
	{"Serle's Triumph",     known_item_properties(10, 65)},
	{"Vorana's Carnage",    known_item_properties(10, 65)},
	{"Cadigan's Epiphany",  known_item_properties(10, 65)},
	{"Katla's Gloom",       known_item_properties(10, 65)},
	{"Thrud's Might",       known_item_properties(10, 65)},
	{"Kolr's Hunt",         known_item_properties(10, 65)},
	{"Aldur's Legacy",      known_item_properties(10, 65)},
	{"Medved's Tending",    known_item_properties(10, 65)},
	{"Uhtred's Sidereus",   known_item_properties(10, 65)},
};

// The Aberration boss drops
inline const std::initializer_list<known_item> augments_runes_aberration = {
	{"Emergent Instinct",    known_item_properties(1, 65)},
	{"Emergent Possibility", known_item_properties(1, 65)},
	{"Emergent Protection",  known_item_properties(1, 65)},
	{"Emergent Vigour",      known_item_properties(1, 65)},
};

inline const std::initializer_list<known_item> augments_abyss = {
	{"Ulaman's Gaze",  known_item_properties(1, 65)},
	{"Kurgal's Gaze",  known_item_properties(1, 65)},
	{"Tecrod's Gaze",  known_item_properties(1, 65)},
	{"Amanamu's Gaze", known_item_properties(1, 65)}
};

inline const std::initializer_list<known_item> augments_soul_cores_regular = {
	{"Soul Core of Atmohua",     known_item_properties(10, 35)},
	{"Soul Core of Azcapa",      known_item_properties(10, 35)},
	{"Soul Core of Cholotl",     known_item_properties(10, 35)},
	{"Soul Core of Citaqualotl", known_item_properties(10, 35)},
	{"Soul Core of Jiquani",     known_item_properties(10, 35)},
	{"Soul Core of Opiloti",     known_item_properties(10, 35)},
	{"Soul Core of Puhuarte",    known_item_properties(10, 35)},
	{"Soul Core of Quipolatl",   known_item_properties(10, 35)},
	{"Soul Core of Tacati",      known_item_properties(10, 35)},
	{"Soul Core of Ticaba",      known_item_properties(10, 35)},
	{"Soul Core of Topotante",   known_item_properties(10, 35)},
	{"Soul Core of Tzamoto",     known_item_properties(10, 35)},
	{"Soul Core of Xopec",       known_item_properties(10, 35)},
	{"Soul Core of Zalatl",      known_item_properties(10, 35)},
	{"Soul Core of Zantipi",     known_item_properties(10, 35)},
};

inline const std::initializer_list<known_item> augments_soul_cores_endgame = {
	{"Atmohua's Soul Core of Retreat",         known_item_properties(10, 65)},
	{"Cholotl's Soul Core of War",             known_item_properties(10, 65)},
	{"Citaqualotl's Soul Core of Foulness",    known_item_properties(10, 65)},
	{"Estazunti's Soul Core of Convalescence", known_item_properties(10, 65)},
	{"Guatelitzi's Soul Core of Endurance",    known_item_properties(10, 65)},
	{"Hayoxi's Soul Core of Heatproofing",     known_item_properties(10, 65)},
	{"Opiloti's Soul Core of Assault",         known_item_properties(10, 65)},
	{"Quipolatl's Soul Core of Flow",          known_item_properties(10, 65)},
	{"Tacati's Soul Core of Affliction",       known_item_properties(10, 65)},
	{"Topotante's Soul Core of Dampening",     known_item_properties(10, 65)},
	{"Tzamoto's Soul Core of Ferocity",        known_item_properties(10, 65)},
	{"Uromoti's Soul Core of Attenuation",     known_item_properties(10, 65)},
	{"Xipocado's Soul Core of Dominion",       known_item_properties(10, 65)},
	{"Xopec's Soul Core of Power",             known_item_properties(10, 65)},
	{"Zalatl's Soul Core of Insulation",       known_item_properties(10, 65)},
};

inline const std::initializer_list<known_item> augments_soul_cores_ancient = {
	{"Citaqualotl's Thesis", known_item_properties(1, 65)},
	{"Guatelitzi's Thesis",  known_item_properties(1, 65)},
	{"Jiquani's Thesis",     known_item_properties(1, 65)},
	{"Quipolatl's Thesis",   known_item_properties(1, 65)},
};

inline const std::initializer_list<known_item> augments_idols_regular = {
	{"Bear Idol",    known_item_properties(10, 35)},
	{"Boar Idol",    known_item_properties(10, 35)},
	{"Cat Idol",     known_item_properties(10, 35)},
	{"Owl Idol",     known_item_properties(10, 35)},
	{"Ox Idol",      known_item_properties(10, 35)},
	{"Primate Idol", known_item_properties(10, 35)},
	{"Snake Idol",   known_item_properties(10, 35)},
	{"Stag Idol",    known_item_properties(10, 35)},
	{"Wolf Idol",    known_item_properties(10, 35)},
	{"Fox Idol",     known_item_properties(10, 65)},
	{"Rabbit Idol",  known_item_properties(10, 65)},
};

inline const std::initializer_list<known_item> augments_idols_endgame = {
	{"Hawk Idol",             known_item_properties(10, 65)},
	{"Idol of Alira",         known_item_properties(10, 65)},
	{"Idol Eeshta",           known_item_properties(10, 65)},
	{"Idol of Egrin",         known_item_properties(10, 65)},
	{"Idol of Eramir",        known_item_properties(10, 65)},
	{"Idol of Greust",        known_item_properties(10, 65)},
	{"Idol of Grold",         known_item_properties(10, 65)},
	{"Idol of Kraityn",       known_item_properties(10, 65)},
	{"Idol of Maxarius",      known_item_properties(10, 65)},
	{"Idol of Oak",           known_item_properties(10, 65)},
	{"Idol of Ralakesh",      known_item_properties(10, 65)},
	{"Idol of Silk",          known_item_properties(10, 65)},
	{"Idol of Sirrius",       known_item_properties(10, 65)},
	{"Idol of the Martyr",    known_item_properties(10, 65)},
	{"Idol of the Pharisee",  known_item_properties(10, 65)},
	{"Idol of the Sycophant", known_item_properties(10, 65)},
	{"Idol of Thruldana",     known_item_properties(10, 65)},
	{"Idol of Yeena",         known_item_properties(10, 65)},
	{"Panther Idol",          known_item_properties(10, 65)},
	{"Stoat Idol",            known_item_properties(10, 65)},
};

inline const std::initializer_list<known_item> augments_idols_ritual = {
	{"Carved Cunning",  known_item_properties(1, 65)},
	{"Carved Majesty",  known_item_properties(1, 65)},
	{"Carved Mischief", known_item_properties(1, 65)},
	{"Carved Tenacity", known_item_properties(1, 65)},
};

inline const std::initializer_list<known_item> augments_other = {
	{"Raven-Touched Shard", known_item_properties(10, 65)}, // 0.5.0, drops from The Raven Trickster
};

inline const std::initializer_list<known_item> omens_legacy = {
	{"Omen of Corruption",              known_item_properties(10,  1)}, // 0.1 - 0.5
	{"Omen of Dextral Alchemy",         known_item_properties(10,  1)}, // 0.1 - 0.3
	{"Omen of Dextral Coronation",      known_item_properties(10,  1)}, // 0.1 - 0.3
	{"Omen of Sinistral Alchemy",       known_item_properties(10,  1)}, // 0.1 - 0.3
	{"Omen of Sinistral Coronation",    known_item_properties(10,  1)}, // 0.1 - 0.3
	{"Omen of Greater Annulment",       known_item_properties(10, 75)}, // 0.1 - 0.3
	{"Omen of Homogenising Coronation", known_item_properties(10, 75)}, // 0.3 - 0.4
	{"Omen of Homogenising Exaltation", known_item_properties(10, 75)}, // 0.3 - 0.4
	// {"Omen of Recombination",           known_item_properties(10, 75)}, // 0.3 - 0.5 (deleted)
};

inline const std::initializer_list<known_item> omens_crafting = {
	{"Omen of Chaotic Rarity",            known_item_properties(10,  1)}, // 0.3
	{"Omen of Chaotic Monsters",          known_item_properties(10,  1)}, // 0.3
	{"Omen of Chaotic Quantity",          known_item_properties(10,  1)}, // 0.3
	{"Omen of Chaotic Effectiveness",     known_item_properties(10,  1)}, // 0.5
	{"Omen of Sinistral Exaltation",      known_item_properties(10,  1)},
	{"Omen of Dextral Exaltation",        known_item_properties(10,  1)},
	{"Omen of Greater Exaltation",        known_item_properties(10,  1)},
	{"Omen of the Ancients",              known_item_properties(10,  1)},
	{"Omen of Whittling",                 known_item_properties(10, 75)},
	{"Omen of Catalysing Exaltation",     known_item_properties(10, 75)},
	{"Omen of Chance",                    known_item_properties(10, 75)},
	{"Omen of Sinistral Crystallisation", known_item_properties(10, 75)},
	{"Omen of Dextral Crystallisation",   known_item_properties(10, 75)},
	{"Omen of Sinistral Erasure",         known_item_properties(10, 79)},
	{"Omen of Dextral Erasure",           known_item_properties(10, 79)},
	{"Omen of Sanctification",            known_item_properties(10, 79)},
	{"Omen of the Blessed",               known_item_properties(10, 79)},
	{"Omen of Sinistral Annulment",       known_item_properties(10, 79)},
	{"Omen of Dextral Annulment",         known_item_properties(10, 79)},
};

inline const std::initializer_list<known_item> omens_non_crafting = {
	{"Omen of Amelioration",        known_item_properties(10, 1)},
	{"Omen of Answered Prayers",    known_item_properties(10, 1)},
	{"Omen of Bartering",           known_item_properties(10, 1)},
	{"Omen of Gambling",            known_item_properties(10, 1)},
	{"Omen of Refreshment",         known_item_properties(10, 1)},
	{"Omen of Reinforcements",      known_item_properties(10, 1)},
	{"Omen of Resurgence",          known_item_properties(10, 1)},
	{"Omen of Secret Compartments", known_item_properties(10, 1)},
	{"Omen of the Hunt",            known_item_properties(10, 1)},
};

inline const std::initializer_list<known_item> omens_abyss = { // 0.3
	{"Omen of Abyssal Echoes",       known_item_properties(10, 1)},
	{"Omen of Dextral Necromancy",   known_item_properties(10, 1)},
	{"Omen of Light",                known_item_properties(10, 1)},
	{"Omen of Putrefaction",         known_item_properties(10, 1)},
	{"Omen of Sinistral Necromancy", known_item_properties(10, 1)},
	{"Omen of the Blackblooded",     known_item_properties(10, 1)},
	{"Omen of the Liege",            known_item_properties(10, 1)},
	{"Omen of the Sovereign",        known_item_properties(10, 1)},
};

inline const std::initializer_list<known_item> omens_expedition = { // 0.5
	{"Aldur's Saga",  known_item_properties(10, 78)},
	{"Medved's Saga", known_item_properties(10, 78)},
	{"Vorana's Saga", known_item_properties(10, 78)},
	{"Uhtred's Saga", known_item_properties(10, 78)},
	{"Olroth's Saga", known_item_properties(10, 78)},
};

// Aldur's Legacy runes
// (not listed, only obtainable through Aldur's Legacy rune crafting)

inline const std::initializer_list<known_item> incubators = { // 0.4.0
	{"Vaal Siphoner", known_item_properties(10, 1)}
};

inline const std::initializer_list<known_item> map_fragments = {
	// 0.1.0
	{"Cowardly Fate",             known_item_properties(1,  1)},
	{"Deadly Fate",               known_item_properties(1,  1)},
	{"Victorious Fate",           known_item_properties(1,  1)},
	{"Simulacrum",                known_item_properties(1, 75)},
	{"An Audience with the King", known_item_properties(1,  1)},
	// 0.3.0
	{"Kulemak's Invitation",    known_item_properties(1, 1)},
	// 0.3.0 until 0.4.0 (removed from the game)
	// {"Idol of Estazunti",      known_item_properties(1, 1)},
	// 0.5.0
	{"The Triskelion Reforged", known_item_properties(1, 1)},
	{"Head of the King",        known_item_properties(1, 1)},
	{"Breachlord Sac",          known_item_properties(1, 1)},
	{"Raven's Reflection",      known_item_properties(1, 1)},
};

inline const std::initializer_list<known_item> pinnacle_keys = {
	// 0.1.0
	{"Ancient Crisis Fragment",   known_item_properties(1, 1)},
	{"Faded Crisis Fragment",     known_item_properties(1, 1)},
	{"Weathered Crisis Fragment", known_item_properties(1, 1)},
	// 0.3.0 until 0.5.0 (now Uber Arbiter of Ash is legacy)
	{"Primary Calamity Fragment",   known_item_properties(1, 1)},
	{"Secondary Calamity Fragment", known_item_properties(1, 1)},
	{"Tertiary Calamity Fragment",  known_item_properties(1, 1)},
	// 0.5.0
	{"Origin Spark",        known_item_properties(1, 1)},
	{"Origin Cradle",       known_item_properties(1, 1)},
	{"Origin Core",         known_item_properties(1, 1)},
	{"Call of the Shadows", known_item_properties(1, 1)},
};

inline const std::initializer_list<known_item> reliquary_keys = {
	{"Azmeri Reliquary Key",                         known_item_properties(1, 35)},
	{"Xesht's Reliquary Key",                        known_item_properties(1, 35)},
	{"Tangmazu's Reliquary Key",                     known_item_properties(1, 35)},
	{"Olroth's Reliquary Key",                       known_item_properties(1, 35)},
	{"The Arbiter's Reliquary Key",                  known_item_properties(1, 35)},
	{"Ritualistic Reliquary Key",                    known_item_properties(1, 35)},
	{"Zarokh's Reliquary Key: Against the Darkness", known_item_properties(1, 35)},
	{"Zarokh's Reliquary Key: Sandstorm Visage",     known_item_properties(1, 35)},
	{"Zarokh's Reliquary Key: Sekhema's Resolve",    known_item_properties(1, 35)},
	{"Zarokh's Reliquary Key: Temporalis",           known_item_properties(1, 35)},
	{"The Trialmaster's Reliquary Key",              known_item_properties(1, 35)},
	{"Twilight Reliquary Key",                       known_item_properties(1, 65)},
};

// specially classed items

// Class == "Breachstone" BaseType == "Breachstone"
inline const std::initializer_list<known_item> breachstones = {
	{"Breachstone", known_item_properties(1, 1)},
};

// Class == "Expedition Logbook" BaseType == "Expedition Logbook"
inline const std::initializer_list<known_item> logbooks = {
	{"Expedition Logbook", known_item_properties(10, 1)},
};

inline const auto known_items = known_items_store::create(
	currency_basic,
	currency_shards,
	currency_essences,
	currency_abyss,
	currency_catalysts,
	currency_emotions,
	currency_expedition,
	currency_vaal,
	augments_runes_lesser,
	augments_runes_regular,
	augments_runes_greater,
	augments_runes_perfect,
	augments_runes_endgame,
	augments_runes_ezomyte_megaliths,
	augments_runes_ward,
	augments_runes_ancient,
	augments_runes_metacraft,
	augments_runes_aberration,
	augments_abyss,
	augments_soul_cores_regular,
	augments_soul_cores_endgame,
	augments_soul_cores_ancient,
	augments_idols_regular,
	augments_idols_endgame,
	augments_idols_ritual,
	augments_other,
	omens_legacy,
	omens_crafting,
	omens_non_crafting,
	omens_abyss,
	omens_expedition,
	incubators,
	map_fragments,
	pinnacle_keys,
	reliquary_keys,
	breachstones,
	logbooks
);

} // namespace poe2

}
