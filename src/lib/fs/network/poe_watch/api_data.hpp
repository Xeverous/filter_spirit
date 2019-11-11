#pragma once

#include <fs/log/logger_fwd.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <system_error>

namespace fs::network::poe_watch
{

struct api_league_data
{
	std::string leagues;
};

struct api_item_price_data
{
	[[nodiscard]] bool save(const boost::filesystem::path& directory, log::logger& logger) const;
	[[nodiscard]] bool load(const boost::filesystem::path& directory, log::logger& logger);

	// https://poe.watch/api
	std::string itemdata_json;
	std::string compact_json;
};

}
