#pragma once

#include <fs/log/monitor.hpp>

#include <string>
#include <filesystem>

namespace fs::network::poe_watch
{

struct api_league_data
{
	std::string leagues;
};

struct api_item_price_data
{
	[[nodiscard]] bool save(const std::filesystem::path& directory, const log::monitor& logger) const;
	[[nodiscard]] bool load(const std::filesystem::path& directory, const log::monitor& logger);

	// https://poe.watch/api
	std::string itemdata_json;
	std::string compact_json;
};

}
