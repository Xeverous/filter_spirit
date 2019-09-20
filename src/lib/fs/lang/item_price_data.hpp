#pragma once

#include <vector>
#include <string>

namespace fs::log { class logger; }

namespace fs::lang
{

// note: this needs to be API-agnostic (currently very centered around poe.watch)
// refactor this (eg remove unused fields) once poe.ninja support is present
struct price_data
{
	double mean = 0;     // arithmetic mean
	double median = 0;   // when sorted, value in the middle
	double mode = 0;     // most repeated value
	double min = 0;      // lower bound of price calculation
	double max = 0;      // upper bound of price calculation
	double exalted = 0;  // same as mean, just in exalted
	double total = 0;    // amount of currently listed items
	double daily = 0;    // amount of new listed items in last 24h
	double current = 0;  // amount of currently listed items that have a price
	double accepted = 0; // amount of accepted offers for price calculation (troll offers are ignored)
};

struct elementary_item
{
	std::string base_type_name;
	price_data price_data;
};

struct base_type_item
{
	int ilvl;
	std::string base_type_name;
	price_data price_data;
};

struct unique_item
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct item_price_data
{
	void log_info(log::logger& logger) const;
	/*
	 * Some items (eg very rare uniques and cards, Harbinger pieces)
	 * may not have associated entry in price data.
	 *
	 * This is not a bug, some items may simply have undetermined value.
	 * We can at most just log them, it's the filter writer's responsibility
	 * to be prepared that not all items have reliable prices.
	 */
	int count_of_items_without_price_data = 0;

	// sorted by price
	std::vector<elementary_item> divination_cards;
	std::vector<elementary_item> prophecies;
	std::vector<elementary_item> essences;
	std::vector<elementary_item> leaguestones;
	std::vector<elementary_item> pieces;
	std::vector<elementary_item> nets;
	std::vector<elementary_item> vials;
	std::vector<elementary_item> fossils;
	std::vector<elementary_item> resonators;
	std::vector<elementary_item> scarabs;

	// sorted by ilvl
	std::vector<base_type_item> bases_without_influence;
	std::vector<base_type_item> bases_shaper;
	std::vector<base_type_item> bases_elder;

	// sorted by price
	std::vector<unique_item> unambiguous_unique_armours;
	std::vector<unique_item> unambiguous_unique_weapons;
	std::vector<unique_item> unambiguous_unique_accessories;
	std::vector<unique_item> unambiguous_unique_jewels;
	std::vector<unique_item> unambiguous_unique_flasks;
	std::vector<unique_item> unambiguous_unique_maps;

	// sorted by base type
	std::vector<unique_item> ambiguous_unique_armours;
	std::vector<unique_item> ambiguous_unique_weapons;
	std::vector<unique_item> ambiguous_unique_accessories;
	std::vector<unique_item> ambiguous_unique_jewels;
	std::vector<unique_item> ambiguous_unique_flasks;
	std::vector<unique_item> ambiguous_unique_maps;
};

}
