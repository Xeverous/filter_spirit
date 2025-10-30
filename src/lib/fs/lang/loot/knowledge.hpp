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
	{"Blessing of Chayula",   known_item_properties(10, 1)}
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
	{"Abrasive Catalyst",     known_item_properties(10,  1)},
	{"Accelerating Catalyst", known_item_properties(10,  1)},
	{"Imbued Catalyst",       known_item_properties(10,  1)},
	{"Intrinsic Catalyst",    known_item_properties(10,  1)},
	{"Noxious Catalyst",      known_item_properties(10,  1)},
	{"Tainted Catalyst",      known_item_properties(10,  1)},
	{"Turbulent Catalyst",    known_item_properties(10,  1)},
	{"Unstable Catalyst",     known_item_properties(10,  1)},
	{"Fertile Catalyst",      known_item_properties(10, 68)},
	{"Prismatic Catalyst",    known_item_properties(10, 68)},
	{"Tempering Catalyst",    known_item_properties(10, 68)}
};

inline const std::initializer_list<known_item> currency_delirium_splinters = { // 3.10
	{"Simulacrum Splinter", known_item_properties(300, 1)}
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
	{"Sacred Crystallised Lifeforce", known_item_properties(10)}
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
	currency_delirium_splinters,
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
	// special drop
	{"Fracturing Orb", known_item_properties(20, 1)}, // (only from Cleansed monsters)
	{"Albino Rhoa Feather", known_item_properties(10, 1)},
	// splinters
	{"Breach Splinter", known_item_properties(300, 1)},
	{"Petition Splinter", known_item_properties(300, 1)},
	{"Runic Splinter", known_item_properties(300, 1)},
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
	{"Perfect Essence of the Mind", known_item_properties(10, 72)}
};

inline const std::initializer_list<known_item> currency_abyss = {
	{"Gnawed Collarbone", known_item_properties(20, 25)},
	{"Gnawed Jawbone", known_item_properties(20, 25)},
	{"Gnawed Rib", known_item_properties(20, 25)},
	{"Preserved Collarbone", known_item_properties(20, 61)},
	{"Preserved Jawbone", known_item_properties(20, 61)},
	{"Preserved Rib", known_item_properties(20, 61)},
	{"Preserved Cranium", known_item_properties(20, 65)},
	{"Preserved Vertebrae", known_item_properties(20, 65)},
	{"Ancient Collarbone", known_item_properties(20, 75)},
	{"Ancient Jawbone", known_item_properties(20, 75)},
	{"Ancient Rib", known_item_properties(20, 75)}
};

inline const std::initializer_list<known_item> currency_catalysts = {
	{"Adaptive Catalyst", known_item_properties(10, 1)},
	{"Carapace Catalyst", known_item_properties(10, 1)},
	{"Chayula's Catalyst", known_item_properties(10, 1)},
	{"Esh's Catalyst", known_item_properties(10, 1)},
	{"Flesh Catalyst", known_item_properties(10, 1)},
	{"Neural Catalyst", known_item_properties(10, 1)},
	{"Reaver Catalyst", known_item_properties(10, 1)},
	{"Sibilant Catalyst", known_item_properties(10, 1)},
	{"Skittering Catalyst", known_item_properties(10, 1)},
	{"Tul's Catalyst", known_item_properties(10, 1)},
	{"Uul-Netol's Catalyst", known_item_properties(10, 1)},
	{"Xoph's Catalyst", known_item_properties(10, 1)}
};

inline const std::initializer_list<known_item> currency_emotions = {
	{"Diluted Liquid Greed", known_item_properties(10, 1)},
	{"Diluted Liquid Guilt", known_item_properties(10, 1)},
	{"Diluted Liquid Ire", known_item_properties(10, 1)},
	{"Liquid Despair", known_item_properties(10, 1)},
	{"Liquid Disgust", known_item_properties(10, 1)},
	{"Liquid Envy", known_item_properties(10, 1)},
	{"Liquid Paranoia", known_item_properties(10, 1)},
	{"Concentrated Liquid Fear", known_item_properties(10, 70)},
	{"Concentrated Liquid Suffering", known_item_properties(10, 75)},
	{"Concentrated Liquid Isolation", known_item_properties(10, 80)},
};

inline const std::initializer_list<known_item> currency_expedition = {
	{"Black Scythe Artifact", known_item_properties(20, 1)},
	{"Broken Circle Artifact", known_item_properties(20, 1)},
	{"Exotic Coinage", known_item_properties(20, 1)},
	{"Order Artifact", known_item_properties(20, 1)},
	{"Sun Artifact", known_item_properties(20, 1)},
};

inline const auto known_items = known_items_store::create(
	currency_basic,
	currency_shards,
	currency_essences,
	currency_abyss,
	currency_catalysts,
	currency_emotions,
	currency_expedition
);

} // namespace poe2

}
