#pragma once

#include <fs/lang/loot/item_database.hpp>
#include <fs/lang/item.hpp>

#include <vector>
#include <utility>
#include <random>

namespace fs::lang::loot {

class item_receiver
{
public:
	virtual ~item_receiver() = default;

	virtual void on_item(const item& i) = 0;
	virtual void on_item(item&& i) = 0;
};

struct range
{
	static range one()
	{
		return { 1, 1 };
	}

	static range one_to(int val)
	{
		return { 1, val };
	}

	int min = 0;
	int max = 0;
};

struct plurality
{
	static plurality only_quantity(range quantity)
	{
		return plurality{quantity, range::one()};
	}

	range quantity;
	range stack_size;
};

struct percent
{
	static constexpr percent min() { return {0}; }
	static constexpr percent max() { return {100}; }

	static constexpr percent never()  { return min(); }
	static constexpr percent always() { return max(); }

	constexpr bool is_valid() const
	{
		return min().value <= value && value <= max().value;
	}

	int value = 0;
};

struct gem_types
{
	bool active = true;
	bool vaal_active = false;
	bool support = true;
	bool awakened_suport = false;
};

struct equippable_item_weights
{
	int sum() const
	{
		return body_armours + helmets + gloves + boots
			+ axes_1h + maces_1h + swords_1h + thrusting_swords + claws + daggers + rune_daggers + wands
			+ axes_2h + maces_2h + swords_2h + staves + warstaves + bows
			+ shields + quivers
			+ amulets + rings + belts
			+ fishing_rods;
	}

	int body_armours = 1000;
	int helmets = 1000;
	int gloves = 1000;
	int boots = 1000;

	// 1-handed
	int axes_1h = 125;
	int maces_1h = 125;
	int swords_1h = 125;
	int thrusting_swords = 125;
	int claws = 125;
	int daggers = 125;
	int rune_daggers = 125;
	int wands = 125;

	// 2-handed
	int axes_2h = 150;
	int maces_2h = 150;
	int swords_2h = 150;
	int staves = 150;
	int warstaves = 150;
	int bows = 150;

	// off-hand
	int shields = 500;
	int quivers = 150;

	// jewellery
	int amulets = 100;
	int rings = 100;
	int belts = 100;

	// other
	int fishing_rods = 0;
};

struct rarity_weights
{
	int sum() const
	{
		return normal + magic + rare;
	}

	int normal = 1000;
	int magic = 1000;
	int rare = 1000;
};

struct item_level_weights
{
	int base = 3000;
	int plus_one = 2000;
	int plus_two = 1000;
};

struct influence_weights
{
	int sum() const
	{
		return none + shaper + elder + crusader + redeemer + hunter + warlord;
	}

	int none     = 10000;
	int shaper   =   200;
	int elder    =   200;
	int crusader =   200;
	int redeemer =   200;
	int hunter   =   200;
	int warlord  =   200;
};

class generator
{
public:
	generator();

	// specific item classes
	void generate_divination_cards  (const item_database& db, item_receiver& receiver, plurality p);
	void generate_resonators        (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_incubators        (const item_database& db, item_receiver& receiver, range quantity, int item_level);
	void generate_metamorph_parts   (const item_database& db, item_receiver& receiver, range quantity, int item_level);
	void generate_unique_pieces     (const item_database& db, item_receiver& receiver, range quantity, int item_level, percent chance_to_corrupt);
	void generate_labyrinth_keys    (const item_database& db, item_receiver& receiver, range quantity);
	void generate_labyrinth_trinkets(const item_database& db, item_receiver& receiver, range quantity);
	void generate_quest_items       (const item_database& db, item_receiver& receiver, range quantity);

	// currency
	void generate_generic_currency       (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_generic_currency_shards(const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_conqueror_orbs         (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_breach_blessings       (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_breach_splinters       (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_legion_splinters       (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_essences               (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_fossils                (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_catalysts              (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_oils                   (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_delirium_orbs          (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_harbinger_scrolls      (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_incursion_vials        (const item_database& db, item_receiver& receiver, plurality p, int area_level);
	void generate_bestiary_nets          (const item_database& db, item_receiver& receiver, plurality p, int area_level);

	void generate_gems(
		const item_database& db,
		item_receiver& receiver,
		range quantity,
		range level,
		range quality,
		int area_level,
		percent chance_to_corrupt,
		gem_types types);

	void generate_equippable_item(
		const item_database& db,
		item_receiver& receiver,
		range quality,
		int item_level,
		percent chance_to_identify,
		percent chance_to_corrupt,
		percent chance_to_mirror,
		equippable_item_weights class_weights,
		influence_weights infl_weights,
		rarity_type rarity_);

	void generate_equippable_items_fixed_weights(
		const item_database& db,
		item_receiver& receiver,
		range quality,
		range quantity,
		int area_level,
		percent chance_to_identify,
		percent chance_to_corrupt,
		percent chance_to_mirror,
		equippable_item_weights class_weights,
		influence_weights infl_weights,
		rarity_weights r_weights,
		item_level_weights ilvl_weights);

	void generate_equippable_items_monster_pack(
		const item_database& db,
		item_receiver& receiver,
		range quality,
		range normal_monsters,
		range magic_monsters,
		range rare_monsters,
		range unique_monsters,
		int area_level,
		double rarity,
		percent chance_to_identify,
		percent chance_to_corrupt,
		percent chance_to_mirror,
		equippable_item_weights class_weights,
		influence_weights infl_weights);

	auto& rng()
	{
		return _rng;
	}

private:
	std::seed_seq _seed_seq;
	std::mt19937 _rng;

	/*
	 * Scratch pads. These are not supposed to hold any meaningful data nor
	 * to have any invariants other than imposed by the vector class template.
	 * Their only purpose is to avoid repetitive allocations when items are being
	 * generated. Free to use for any member function.
	 */
	std::vector<std::size_t> _sp_indexes;
	std::vector<std::pair<int, rarity_type>> _temp_ilvl_rarity;
};

}
