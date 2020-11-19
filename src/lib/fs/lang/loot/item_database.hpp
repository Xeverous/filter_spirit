#pragma once

#include <fs/log/logger.hpp>

#include <string_view>
#include <string>
#include <vector>
#include <optional>

namespace fs::lang::loot {

struct elementary_item
{
	std::string metadata_path;
	std::string name;
	int drop_level = 1;
	int width = 1;
	int height = 1;
};

struct currency_item : elementary_item
{
	int max_stack_size = 1;
};

struct equippable_item : elementary_item
{
	int max_sockets = 0;
	bool is_atlas_base_type = false;
};

struct gem : elementary_item
{
	int max_level = 20;
};

struct map : elementary_item
{
	// int map_tier; // TODO: Metadata does not contain such information, parse MapSeriesTiers.dat
};

struct resonator : currency_item
{
	int delve_sockets = 1;
};

struct unique_piece : elementary_item
{
	std::string piece_name; // "Nth Piece of X"
};

struct currency_item_database
{
	std::vector<currency_item> generic;
	std::vector<currency_item> generic_shards;
	std::vector<currency_item> conqueror_orbs;
	std::vector<currency_item> breach_blessings;
	std::vector<currency_item> breach_splinters;
	std::vector<currency_item> legion_splinters;
	std::vector<currency_item> essences;
	std::vector<currency_item> fossils;
	std::vector<currency_item> catalysts;
	std::vector<currency_item> oils;
	std::vector<currency_item> delirium_orbs;
	std::vector<currency_item> harbinger_scrolls;
	std::vector<currency_item> incursion_vials;
	std::vector<currency_item> bestiary_nets;

	std::optional<currency_item> simulacrum_splinter;
	std::optional<currency_item> remnant_of_corruption;
	std::optional<currency_item> bestiary_orb;
	std::optional<currency_item> albino_rhoa_feather;
};

struct equippable_item_database
{
	// main parts
	std::vector<equippable_item> body_armours;
	std::vector<equippable_item> helmets;
	std::vector<equippable_item> gloves;
	std::vector<equippable_item> boots;

	// 1-handed
	std::vector<equippable_item> axes_1h;
	std::vector<equippable_item> maces_1h;
	std::vector<equippable_item> swords_1h;
	std::vector<equippable_item> thrusting_swords;
	std::vector<equippable_item> claws;
	std::vector<equippable_item> daggers;
	std::vector<equippable_item> rune_daggers;
	std::vector<equippable_item> wands;

	// 2-handed
	std::vector<equippable_item> axes_2h;
	std::vector<equippable_item> maces_2h;
	std::vector<equippable_item> swords_2h;
	std::vector<equippable_item> staves;
	std::vector<equippable_item> warstaves;
	std::vector<equippable_item> bows;

	// off-hand
	std::vector<equippable_item> shields;
	std::vector<equippable_item> quivers;

	// jewellery
	std::vector<equippable_item> amulets;
	std::vector<equippable_item> rings;
	std::vector<equippable_item> belts;
	std::optional<equippable_item> stygian_vise; // the only base with an abyss socket

	// other
	std::vector<equippable_item> talismans;
	std::vector<equippable_item> fishing_rods;
};

struct flask_database
{
	std::vector<elementary_item> life_flasks;
	std::vector<elementary_item> mana_flasks;
	std::vector<elementary_item> hybrid_flasks;
	std::vector<elementary_item> utility_flasks;
	std::vector<elementary_item> critical_utility_flasks;
};

struct jewel_database
{
	std::vector<elementary_item> generic_jewels;
	std::vector<elementary_item> abyss_jewels;
	std::vector<elementary_item> cluster_jewels;
};

struct gem_database
{
	std::vector<gem> active_gems;
	std::vector<gem> vaal_active_gems;
	std::vector<gem> support_gems;
	std::vector<gem> awakened_support_gems;
};

struct map_fragments_database
{
	std::vector<elementary_item> ordinary_scarabs;
	std::vector<elementary_item> winged_scarabs;
	std::vector<elementary_item> lures;

	std::vector<elementary_item> shaper_fragments;
	std::vector<elementary_item> elder_fragments;
	std::vector<elementary_item> uber_elder_fragments;
	std::vector<elementary_item> atziri_fragments;
	std::vector<elementary_item> uber_atziri_fragments;
	std::vector<elementary_item> prophecy_fragments; // aka Pale Court keys
	std::vector<elementary_item> legion_fragments; // aka Emblems
	std::vector<elementary_item> breachstones;

	std::optional<elementary_item> simulacrum;
	std::optional<elementary_item> divine_vessel;
	std::optional<elementary_item> labyrinth_offering;
	std::vector<elementary_item> labyrinth_upgraded_offerings;
};

// how to generete the data using RePoE:
// https://www.reddit.com/r/pathofexiledev/comments/i3p4ly/drop_level_property_of_various_items/

// stores all information required to generate example items
struct item_database
{
	bool parse(std::string_view items_metadata_json, log::logger& logger);

	void print_stats(log::logger& logger) const;

	currency_item_database currency;
	equippable_item_database equipment;
	flask_database flasks;
	jewel_database jewels;
	gem_database gems;
	map_fragments_database map_fragments;

	std::vector<map> maps;

	std::vector<elementary_item> quest_items;
	std::vector<elementary_item> incubators;
	std::vector<resonator> resonators;
	std::vector<currency_item> divination_cards;
	std::vector<elementary_item> metamorph_parts;
	std::vector<unique_piece> unique_pieces;

	std::optional<elementary_item> incursion_key;
	std::optional<elementary_item> incursion_bomb;

	std::vector<elementary_item> labyrinth_keys;
	std::vector<elementary_item> labyrinth_trinkets;

	std::vector<elementary_item> leaguestones;
	std::vector<elementary_item> reliquary_keys;
};

log::message_stream& operator<<(log::message_stream& stream, const item_database& db);

}
