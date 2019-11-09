#include <fs/lang/item_price_metadata.hpp>
#include <fs/utility/dump_json.hpp>
#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

#include <nlohmann/json.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <utility>

namespace
{

constexpr auto filename_metadata = "metadata.json";

constexpr auto field_league_name = "league_name";
constexpr auto field_data_source = "data_source";
constexpr auto field_download_date = "download_date";

}

namespace fs::lang
{

bool item_price_metadata::save(const boost::filesystem::path& directory, fs::log::logger& logger) const
{
	nlohmann::json json = {
		{field_league_name, league_name},
		{field_data_source, to_string(data_source)},
		{field_download_date, boost::posix_time::to_iso_string(download_date)}
	};

	std::error_code ec = utility::save_file(directory / filename_metadata, utility::dump_json(json));

	if (ec)
		logger.error() << "failed to save item price metadata: " << ec.message();

	return !static_cast<bool>(ec);
}

bool item_price_metadata::load(const boost::filesystem::path& directory, fs::log::logger& logger)
{
	std::error_code ec;
	std::string file_content = utility::load_file(directory / filename_metadata, ec);

	if (ec) {
		logger.error() << "failed to load metadata file: " << ec.message();
		return false;
	}

	auto json = nlohmann::json::parse(file_content);

	// create a separate data instance to implement strong exception guuarantee
	item_price_metadata data;
	data.league_name = json.at(field_league_name).get<std::string>();

	if (
		std::optional<data_source_type> data_source = lang::from_string(
			json.at(field_data_source).get_ref<const std::string&>());
		data_source)
	{
		data.data_source = *data_source;
	}
	else {
		logger.error() << "failed to parse data source from metadata file";
		return false;
	}

	data.download_date = boost::posix_time::from_iso_string(
		json.at(field_download_date).get_ref<const std::string&>());
	if (data.download_date.is_special()) {
		logger.error() << "invalid date in metadata file";
		return false;
	}

	*this = std::move(data);
	return true;
}

}
