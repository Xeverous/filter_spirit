#include <fs/network/poe_watch/api_data.hpp>
#include <fs/utility/file.hpp>

namespace
{

constexpr auto filename_compact = "compact.json";
constexpr auto filename_itemdata = "itemdata.json";

}

namespace fs::network::poe_watch
{

std::error_code api_item_price_data::save(const boost::filesystem::path& directory) const
{
	std::error_code ec = utility::save_file(directory / filename_compact, compact_json);

	if (ec)
		return ec;

	return utility::save_file(directory / filename_itemdata, itemdata_json);
}

std::error_code api_item_price_data::load(const boost::filesystem::path& directory)
{
	std::error_code ec;
	compact_json = utility::load_file(directory / filename_compact, ec);

	if (ec)
		return ec;

	itemdata_json = utility::load_file(directory / filename_itemdata, ec);
	return ec;
}

}
