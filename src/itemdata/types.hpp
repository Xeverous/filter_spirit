#pragma once
#include <vector>
#include <string>

namespace fs::itemdata
{

struct league
{
	int id;
	std::string name;         // name used by the API
	std::string display_name; // name intended for display to users
};

struct price_data
{
	double mean;     // arithmetic mean
	double median;   // when sorted, value in the middle
	double mode;     // most repeated value
	double min;      // lower bound of price calculation
	double max;      // upper bound of price calculation
	double exalted;  // same as mean, just in exalted
	double total;    // amount of currently listed items
	double daily;    // amount of new listed items in last 24h
	double current;  // amount of currently listed items that have a price
	double accepted; // amount of accepted offers for price calculation (troll offers are ignored)
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
	// sorted by price
	std::vector<elementary_item> divination_cards;
	std::vector<elementary_item> prophecies;

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
