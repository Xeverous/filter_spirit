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

struct divination_card
{
	std::string base_type_name;
	price_data price_data;
};

struct prophecy
{
	std::string base_type_name;
	price_data price_data;
};

struct base_without_influence
{
	int ilvl;
	std::string base_type_name;
	price_data price_data;
};

struct base_shaper
{
	int ilvl;
	std::string base_type_name;
	price_data price_data;
};

struct base_elder
{
	int ilvl;
	std::string base_type_name;
	price_data price_data;
};

struct unique_armour
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct unique_weapon
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct unique_accessory
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct unique_jewel
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct unique_flask
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct unique_map
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct relic_item
{
	std::string name;
	std::string base_type_name;
	price_data price_data;
};

struct item_price_data
{
	std::vector<divination_card> divination_cards;               // sorted by price
	std::vector<prophecy> prophecies;                            // sorted by price
	std::vector<base_without_influence> bases_without_influence; // sorted by ilvl
	std::vector<base_shaper> bases_shaper;                       // sorted by ilvl
	std::vector<base_elder> bases_elder;                         // sorted by ilvl

	std::vector<unique_armour> unique_armours;                   // sorted by price
	std::vector<unique_weapon> unique_weapons;                   // sorted by price
	std::vector<unique_accessory> unique_accessories;            // sorted by price
	std::vector<unique_jewel> unique_jewels;                     // sorted by price
	std::vector<unique_flask> unique_flasks;                     // sorted by price
	std::vector<unique_map> unique_maps;                         // sorted by price

	std::vector<relic_item> relic_items;                         // sorted by price
};

}
