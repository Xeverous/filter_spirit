#pragma once

#include <boost/container/static_vector.hpp>

#include <string>
#include <initializer_list>
#include <optional>

namespace fs::lang::kl {

struct stackable_item
{
	std::string name;
	int max_stack_size;
	int drop_level;
};

inline const std::initializer_list<stackable_item> currency_basic = {
	// (by drop level, then alphabetical)
	{"Armourer's Scrap",                 40,  1},
	{"Blacksmith's Whetstone",           20,  1},
	{"Orb of Augmentation",              30,  1},
	{"Orb of Transmutation",             40,  1},
	{"Scroll of Wisdom",                 40,  1},
	{"Chromatic Orb",                    20,  2},
	{"Glassblower's Bauble",             20,  2},
	{"Orb of Alchemy",                   20,  2},
	{"Orb of Alteration",                20,  2},
	{"Orb of Chance",                    20,  2},
	{"Orb of Regret",                    40,  2},
	{"Portal Scroll",                    40,  2},
	{"Jeweller's Orb",                   20,  8},
	{"Orb of Fusing",                    20,  8},
	{"Chaos Orb",                        20, 12},
	{"Gemcutter's Prism",                20, 12},
	{"Orb of Scouring",                  30, 12},
	{"Regal Orb",                        20, 12},
	{"Vaal Orb",                         20, 12},
	{"Orb of Binding",                   20, 25},
	{"Blessed Orb",                      20, 35},
	{"Divine Orb",                       20, 35},
	{"Exalted Orb",                      20, 35},
	{"Mirror of Kalandra",               10, 35},
	{"Orb of Annulment",                 20, 35},
	{"Sacred Orb",                       10, 35},
	{"Enkindling Orb",                   20, 40},
	{"Hinekora's Lock",                  10, 40},
	{"Instilling Orb",                   20, 40},
	{"Rogue's Marker",                50000, 48},
	{"Stacked Deck",                     20, 50},
	{"Cartographer's Chisel",            20, 52},
	// Lesser Eldritch are here - they can drop on T1 maps without Altars (wiki also puts them here)
	{"Lesser Eldritch Ember",            10, 68},
	{"Lesser Eldritch Ichor",            10, 68},
	{"Orb of Horizons",                  20, 68},
	{"Blighted Scouting Report",         20, 69},
	{"Comprehensive Scouting Report",    20, 69},
	{"Delirious Scouting Report",        20, 69},
	{"Explorer's Scouting Report",       20, 69},
	{"Operative's Scouting Report",      20, 69},
	{"Singular Scouting Report",         20, 69},
	{"Vaal Scouting Report",             20, 69},
	{"Orb of Unmaking",                  40, 70},
	{"Influenced Scouting Report",       20, 81},
	{"Otherwordly Scouting Report",      20, 81},
};

inline const std::initializer_list<stackable_item> currency_shards = {
	// (by drop level, then alphabetical)
	{"Alchemy Shard",       20,  1},
	{"Alteration Shard",    20,  1},
	{"Scroll Fragment",      5,  1},
	{"Transmutation Shard", 20,  1},
	{"Chaos Shard",         20,  8},
	{"Engineer's Shard",    20,  8},
	{"Regal Shard",         20, 12},
	{"Binding Shard",       20, 25},
	{"Ancient Shard",       20, 35},
	{"Annulment Shard",     20, 35},
	{"Exalted Shard",       20, 35},
	{"Fracturing Shard",    20, 35},
	{"Mirror Shard",        20, 35},
	{"Harbinger's Shard",   20, 52},
	{"Horizon Shard",       20, 52},
};

inline const std::initializer_list<stackable_item> currency_atlas = {
	// (by drop level, then alphabetical)
	{"Awakener's Orb",                  10,  1}, // drops from Sirus
	{"Crusader's Exalted Orb",          20,  1}, // drops from Baran
	{"Hunter's Exalted Orb",            20,  1}, // drops from Al-Hezmin

	// drops from any Maven's Inviation (any Crucible) or witnessed map bosses
	{"Maven's Chisel of Avarice",       20,  1},
	{"Maven's Chisel of Divination",    20,  1},
	{"Maven's Chisel of Procurement",   20,  1},
	{"Maven's Chisel of Proliferation", 20,  1},
	{"Maven's Chisel of Scarabs",       20,  1},

	{"Orb of Conflict",                 10,  1}, // drops from Maven and Invitations
	{"Orb of Dominance",                10,  1}, // Elder, Uber Elder, Shaper, Sirus

	// Incarnation bosses
	{"Orb of Intention",                10,  1},
	{"Orb of Remembrance",              10,  1},
	{"Orb of Unravelling",              10,  1},

	{"Redeemer's Exalted Orb",          20,  1}, // drops from Veritania
	{"Warlord's Exalted Orb",           20,  1}, // drops from Drox

	// Exarch/Eater monsters or their mini-bosses
	{"Greater Eldritch Ember",          10, 73},
	{"Greater Eldritch Ichor",          10, 73},
	{"Eldritch Chaos Orb",              20, 78},
	{"Eldritch Exalted Orb",            20, 78},
	{"Eldritch Orb of Annulment",       20, 78},
	{"Grand Eldritch Ember",            10, 78},
	{"Grand Eldritch Ichor",            10, 78},

	// Exarch/Eater mini-bosses
	{"Exceptional Eldritch Ember",      10, 81},
	{"Exceptional Eldritch Ichor",      10, 81},

	// drops from any Maven's Inviation (any Crucible)
	{"Crescent Splinter",               10, 83},
};

// note: drop-level for Essences has weak meaning as corruption
// and atlas passives can produce better tiers than original
// TODO verify drop-level by in-game testing
inline const std::initializer_list<stackable_item> currency_essences = {
	// Group A
	{"Whispering Essence of Greed", 9,  1},
	{"Muttering Essence of Greed",  9, 12},
	{"Weeping Essence of Greed",    9, 30},
	{"Wailing Essence of Greed",    9, 48},
	{"Screaming Essence of Greed",  9, 68},
	{"Shrieking Essence of Greed",  9, 73},
	{"Deafening Essence of Greed",  9, 73},

	{"Whispering Essence of Contempt", 9,  1},
	{"Muttering Essence of Contempt",  9, 12},
	{"Weeping Essence of Contempt",    9, 30},
	{"Wailing Essence of Contempt",    9, 48},
	{"Screaming Essence of Contempt",  9, 68},
	{"Shrieking Essence of Contempt",  9, 73},
	{"Deafening Essence of Contempt",  9, 73},

	{"Whispering Essence of Hatred", 9,  1},
	{"Muttering Essence of Hatred",  9, 12},
	{"Weeping Essence of Hatred",    9, 30},
	{"Wailing Essence of Hatred",    9, 48},
	{"Screaming Essence of Hatred",  9, 68},
	{"Shrieking Essence of Hatred",  9, 73},
	{"Deafening Essence of Hatred",  9, 73},

	{"Whispering Essence of Woe", 9,  1},
	{"Muttering Essence of Woe",  9, 12},
	{"Weeping Essence of Woe",    9, 30},
	{"Wailing Essence of Woe",    9, 48},
	{"Screaming Essence of Woe",  9, 68},
	{"Shrieking Essence of Woe",  9, 73},
	{"Deafening Essence of Woe",  9, 73},

	// Group B
	{"Muttering Essence of Fear",  9, 12},
	{"Weeping Essence of Fear",    9, 30},
	{"Wailing Essence of Fear",    9, 48},
	{"Screaming Essence of Fear",  9, 68},
	{"Shrieking Essence of Fear",  9, 73},
	{"Deafening Essence of Fear",  9, 73},

	{"Muttering Essence of Anger",  9, 12},
	{"Weeping Essence of Anger",    9, 30},
	{"Wailing Essence of Anger",    9, 48},
	{"Screaming Essence of Anger",  9, 68},
	{"Shrieking Essence of Anger",  9, 73},
	{"Deafening Essence of Anger",  9, 73},

	{"Muttering Essence of Torment",  9, 12},
	{"Weeping Essence of Torment",    9, 30},
	{"Wailing Essence of Torment",    9, 48},
	{"Screaming Essence of Torment",  9, 68},
	{"Shrieking Essence of Torment",  9, 73},
	{"Deafening Essence of Torment",  9, 73},

	{"Muttering Essence of Sorrow",  9, 12},
	{"Weeping Essence of Sorrow",    9, 30},
	{"Wailing Essence of Sorrow",    9, 48},
	{"Screaming Essence of Sorrow",  9, 68},
	{"Shrieking Essence of Sorrow",  9, 73},
	{"Deafening Essence of Sorrow",  9, 73},

	// Group C
	{"Weeping Essence of Rage",    9, 30},
	{"Wailing Essence of Rage",    9, 48},
	{"Screaming Essence of Rage",  9, 68},
	{"Shrieking Essence of Rage",  9, 73},
	{"Deafening Essence of Rage",  9, 73},

	{"Weeping Essence of Suffering",    9, 30},
	{"Wailing Essence of Suffering",    9, 48},
	{"Screaming Essence of Suffering",  9, 68},
	{"Shrieking Essence of Suffering",  9, 73},
	{"Deafening Essence of Suffering",  9, 73},

	{"Weeping Essence of Wrath",    9, 30},
	{"Wailing Essence of Wrath",    9, 48},
	{"Screaming Essence of Wrath",  9, 68},
	{"Shrieking Essence of Wrath",  9, 73},
	{"Deafening Essence of Wrath",  9, 73},

	{"Weeping Essence of Doubt",    9, 30},
	{"Wailing Essence of Doubt",    9, 48},
	{"Screaming Essence of Doubt",  9, 68},
	{"Shrieking Essence of Doubt",  9, 73},
	{"Deafening Essence of Doubt",  9, 73},

	// Group D
	{"Wailing Essence of Loathing",    9, 48},
	{"Screaming Essence of Loathing",  9, 68},
	{"Shrieking Essence of Loathing",  9, 73},
	{"Deafening Essence of Loathing",  9, 73},

	{"Wailing Essence of Zeal",    9, 48},
	{"Screaming Essence of Zeal",  9, 68},
	{"Shrieking Essence of Zeal",  9, 73},
	{"Deafening Essence of Zeal",  9, 73},

	{"Wailing Essence of Anguish",    9, 48},
	{"Screaming Essence of Anguish",  9, 68},
	{"Shrieking Essence of Anguish",  9, 73},
	{"Deafening Essence of Anguish",  9, 73},

	{"Wailing Essence of Spite",    9, 48},
	{"Screaming Essence of Spite",  9, 68},
	{"Shrieking Essence of Spite",  9, 73},
	{"Deafening Essence of Spite",  9, 73},

	// Group E
	{"Screaming Essence of Scorn",  9, 68},
	{"Shrieking Essence of Scorn",  9, 73},
	{"Deafening Essence of Scorn",  9, 73},

	{"Screaming Essence of Envy",  9, 68},
	{"Shrieking Essence of Envy",  9, 73},
	{"Deafening Essence of Envy",  9, 73},

	{"Screaming Essence of Misery",  9, 68},
	{"Shrieking Essence of Misery",  9, 73},
	{"Deafening Essence of Misery",  9, 73},

	{"Screaming Essence of Dread",  9, 68},
	{"Shrieking Essence of Dread",  9, 73},
	{"Deafening Essence of Dread",  9, 73},

	// Group F (corruption only)
	{"Essence of Insanity", 9, 73},
	{"Essence of Horror",   9, 73},
	{"Essence of Delirium", 9, 73},
	{"Essence of Hysteria", 9, 73},

	// other
	{"Remnant of Corruption", 9, 1}
};

inline const std::initializer_list<stackable_item> currency_breach = {

};

	// breach
	// legion
	// harbinger (better scrolls)
	// incursion (vials)
	// delve
	// blight
	// delirium
	// heist
	// ritual
	// ultimatum
	// expedition
	// beyond (tainted)
	// kalandra (reflecting mist)
	// runegrafts
	// tattoos

class stackable_item_collection
{
public:
	static constexpr auto max_groups = 4;
	using group_type = std::pair<const stackable_item*, const stackable_item*>;
	using container_type = boost::container::static_vector<group_type, max_groups>;

	stackable_item_collection() = default;

	stackable_item_collection(std::initializer_list<group_type> groups)
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

		const stackable_item& operator*() const
		{
			return (m_groups_it->first)[m_ingroup_idx];
		}

		const stackable_item* operator->() const
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
		for (const stackable_item& itm : *this) {
			if (itm.name == item_name)
				return itm.max_stack_size;
		}

		return std::nullopt;
	}

private:
	container_type m_groups;
};

inline const stackable_item_collection group_currency = {{
	std::make_pair(currency_basic .begin(), currency_basic .end()),
	std::make_pair(currency_shards.begin(), currency_shards.end()),
	std::make_pair(currency_atlas .begin(), currency_atlas .end()),
}};

}
