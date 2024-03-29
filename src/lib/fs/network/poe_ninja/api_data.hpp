#pragma once

#include <fs/log/logger.hpp>

#include <string>
#include <filesystem>

namespace fs::network::poe_ninja
{

// JSON file contents returned by querying poe.ninja
// https://poe.ninja/swagger
struct api_item_price_data
{
	[[nodiscard]] bool save(const std::filesystem::path& directory, log::logger& logger) const;
	[[nodiscard]] bool load(const std::filesystem::path& directory, log::logger& logger);

	// poe.ninja/api/data/currencyoverview
	std::string currency;
	std::string fragment;

	// poe.ninja/api/data/itemoverview
	std::string delirium_orb;
	std::string oil;
	std::string incubator;
	std::string scarab;
	std::string fossil;
	std::string resonator;
	std::string essence;
	std::string divination_card;
	std::string skill_gem;
	std::string base_type;
	std::string tattoo;
	std::string unique_map;
	std::string map;
	std::string unique_jewel;
	std::string unique_flask;
	std::string unique_weapon;
	std::string unique_armour;
	std::string unique_accessory;
	std::string vial;
};

}
