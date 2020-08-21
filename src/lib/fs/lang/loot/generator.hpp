#pragma once

#include <fs/lang/loot/item_database.hpp>
#include <fs/lang/item.hpp>

#include <vector>
#include <utility>
#include <random>

namespace fs::lang::loot {

class item_inserter
{
public:
	item_inserter(std::vector<item>& v)
	: _vector(v)
	{
	}

	void insert(const item& i)
	{
		_vector.push_back(i);
	}

	void insert(item&& i)
	{
		_vector.push_back(std::move(i));
	}

private:
	std::vector<item>& _vector;
};

enum class stack_param
{
	single, // always 1
	any,    // 1 - N
	full    // always N
};

class generator
{
public:
	generator();

	// low-level generation functions - use these to compose more complex loots

	void generate_cards(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_incubators(const item_database& db, item_inserter inserter, int count, int item_level);
	void generate_quest_items(const item_database& db, item_inserter inserter, int count);
	void generate_resonators(const item_database& db, item_inserter inserter, int count);
	void generate_metamorph_parts(const item_database& db, item_inserter inserter, int count);
	void generate_unique_pieces(const item_database& db, item_inserter inserter, int count, int item_level);
	void generate_labyrinth_keys(const item_database& db, item_inserter inserter, int count);
	void generate_labyrinth_trinkets(const item_database& db, item_inserter inserter, int count);

	void generate_generic_currency(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_generic_currency_shards(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_conqueror_orbs(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_breach_blessings(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_breach_splinters(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_legion_splinters(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_essences(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_fossils(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_catalysts(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_oils(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_delirium_orbs(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_harbinger_scrolls(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_incursion_vials(const item_database& db, item_inserter inserter, int count, stack_param stacking);
	void generate_bestiary_nets(const item_database& db, item_inserter inserter, int count, stack_param stacking);

	void generate_non_unique_equippable_item(
		const item_database& db,
		item_inserter inserter,
		int item_level,
		rarity_type rarity_,
		bool allow_atlas_bases);

	void generate_monster_loot(
		const item_database& db,
		item_inserter inserter,
		double rarity,
		double quantity,
		int item_level,
		bool allow_atlas_bases);

	void generate_monster_pack_loot(
		const item_database& db,
		item_inserter inserter,
		double rarity,
		double quantity,
		int area_level,
		bool allow_atlas_bases,
		int num_normal_monsters,
		int num_magic_monsters,
		int num_rare_monsters,
		int num_unique_monsters);


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
