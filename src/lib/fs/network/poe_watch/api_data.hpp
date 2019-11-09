#pragma once

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
	[[nodiscard]] std::error_code save(const boost::filesystem::path& directory) const;
	[[nodiscard]] std::error_code load(const boost::filesystem::path& directory);

	// https://poe.watch/api
	std::string itemdata_json;
	std::string compact_json;
};

}
