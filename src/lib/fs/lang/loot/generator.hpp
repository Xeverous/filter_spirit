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

	constexpr bool is_valid() const
	{
		return min().value <= value && value <= max().value;
	}

	int value = 0;
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

	void generate_non_unique_equippable_item(
		const item_database& db,
		item_receiver& receiver,
		int item_level,
		rarity_type rarity_,
		bool allow_atlas_bases);

	void generate_monster_loot(
		const item_database& db,
		item_receiver& receiver,
		double rarity,
		double quantity,
		int item_level,
		bool allow_atlas_bases);

	void generate_monster_pack_loot(
		const item_database& db,
		item_receiver& receiver,
		double rarity,
		double quantity,
		int area_level,
		bool allow_atlas_bases,
		int num_normal_monsters,
		int num_magic_monsters,
		int num_rare_monsters,
		int num_unique_monsters);

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
};

}
