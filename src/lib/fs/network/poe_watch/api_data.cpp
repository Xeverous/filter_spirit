#include <fs/network/poe_watch/api_data.hpp>
#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

#include <utility>

namespace
{

constexpr auto filename_compact = "compact.json";
constexpr auto filename_itemdata = "itemdata.json";

}

namespace fs::network::poe_watch
{

bool api_item_price_data::save(const boost::filesystem::path& directory, log::logger& logger) const
{
	return utility::save_file(directory / filename_compact, compact_json, logger)
		&& utility::save_file(directory / filename_itemdata, itemdata_json, logger);
}

bool api_item_price_data::load(const boost::filesystem::path& directory, log::logger& logger)
{
	std::optional<std::string> file_contents = utility::load_file(directory / filename_compact, logger);

	if (file_contents) {
		compact_json = std::move(*file_contents);
	}
	else {
		return false;
	}

	file_contents = utility::load_file(directory / filename_itemdata, logger);

	if (file_contents) {
		itemdata_json = std::move(*file_contents);
	}
	else {
		return false;
	}

	return true;
}

}
