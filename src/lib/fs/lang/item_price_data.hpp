#pragma once

#include <fs/log/logger_fwd.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>

namespace fs::lang
{

struct price_data
{
	double chaos_value;
	bool is_low_confidence;
};

struct elementary_item
{
	price_data price;
	std::string name;
};

struct divination_card : elementary_item
{
	divination_card(elementary_item ei, int stack_size)
	: elementary_item(std::move(ei)), stack_size(stack_size) {}

	divination_card(price_data price, std::string name, int stack_size)
	: elementary_item{price, std::move(name)}, stack_size(stack_size) {}

	int stack_size;
};

struct gem : elementary_item
{
	gem(elementary_item ei, int level, int quality, bool is_corrupted)
	: elementary_item(std::move(ei)), level(level), quality(quality), is_corrupted(is_corrupted) {}

	int level;
	int quality;
	bool is_corrupted;
};

enum class influence_type { none, shaper, elder };

struct base : elementary_item
{
	base(elementary_item ei, int item_level, influence_type influence)
	: elementary_item(std::move(ei)), item_level(item_level), influence(influence) {}

	int item_level;
	influence_type influence;
};

bool is_undroppable_unique(std::string_view name) noexcept;

// unlinked uniques
struct unique_item_price_data
{
	void add_item(std::string base_type, elementary_item item_info);

	// maps base type name to unique item name
	// (only 1 unique on the given base)
	std::unordered_map<std::string, elementary_item> unambiguous;
	// maps base type name to unique item names
	// (contains multiple entries per base type)
	std::unordered_multimap<std::string, elementary_item> ambiguous;
};

struct item_price_metadata;

struct item_price_data
{
	[[nodiscard]] bool
	load_and_parse(
		const item_price_metadata& metadata,
		const std::string& directory_path,
		log::logger& logger);

	std::vector<divination_card> divination_cards;

	std::vector<elementary_item> oils;
	std::vector<elementary_item> incubators;
	std::vector<elementary_item> essences;
	std::vector<elementary_item> fossils;
	std::vector<elementary_item> prophecies;
	std::vector<elementary_item> resonators;
	std::vector<elementary_item> scarabs;
	std::vector<elementary_item> helmet_enchants;

	std::vector<gem> gems;

	std::vector<base> bases;

	unique_item_price_data unique_eq; // jewellery, body parts, weapons
	unique_item_price_data unique_flasks;
	unique_item_price_data unique_jewels;
	unique_item_price_data unique_maps;
};

log::logger_wrapper& operator<<(log::logger_wrapper& logger, const item_price_data& ipd);

}
