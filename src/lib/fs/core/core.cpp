#include <fs/core/core.hpp>
#include <fs/utility/file.hpp>
#include <fs/generator/generate_filter.hpp>
#include <fs/network/poe_watch/api.hpp>
#include <fs/network/poe_watch/parse_json.hpp> // TODO avoid including this
#include <fs/log/logger.hpp>

#include <fstream>

namespace fs::core
{

bool generate_item_filter(
	const lang::item_price_data& item_price_data,
	const std::string& source_filepath,
	const std::string& output_filepath,
	bool print_ast,
	log::logger& logger)
{
	std::optional<std::string> source_file_content = utility::load_file(source_filepath);

	if (!source_file_content)
	{
		logger.error() << "can not open input file";
		return false;
	}

	std::optional<std::string> filter_content = generator::generate_filter(
		*source_file_content,
		item_price_data,
		generator::options{print_ast},
		generator::metadata{},
		logger);

	if (!filter_content)
		return false;

	std::ofstream output_file(output_filepath);
	if (!output_file.good())
	{
		logger.error() << "can not open output file";
		return false;
	}

	output_file << *filter_content;
	return true;
}

void list_leagues(log::logger& logger)
{
	std::future<std::string> leagues_future = network::poe_watch::async_download_leagues(logger);
	const auto leagues_str = leagues_future.get();
	const std::vector<lang::league> leagues = network::poe_watch::parse_leagues(leagues_str);

	logger.begin_info_message();
	logger.add("available leagues:\n");

	// print leaue.name because that's the name that the API expects
	// printing different name style would confuse users - these names are quered
	// to be later to the API as league name
	for (const lang::league& league : leagues)
		logger << league.name << "\n";

	logger.end_message();
}

lang::item_price_data download_item_price_data(const std::string& league_name, log::logger& logger)
{
	try
	{
		std::future<network::poe_watch::item_price_data> ipd_future =
			network::poe_watch::async_download_item_price_data(league_name, logger);
		const network::poe_watch::item_price_data ipd = ipd_future.get();
		return network::poe_watch::parse_item_price_data(ipd, logger);
	}
	catch (const std::exception& e)
	{
		logger.error() << e.what();
		throw;
	}
}

std::optional<lang::item_price_data> load_item_price_data(const std::string& directory_path, log::logger& logger)
{
	std::optional<std::string> compact = utility::load_file(directory_path + "/compact.json");

	if (!compact)
	{
		logger.error() << "can not open compact.json";
		return std::nullopt;
	}

	std::optional<std::string> itemdata = utility::load_file(directory_path + "/itemdata.json");

	if (!compact)
	{
		logger.error() << "can not open itemdata.json";
		return std::nullopt;
	}

	const std::string& compact_json = *compact;
	const std::string& itemdata_json = *itemdata;
	return network::poe_watch::parse_item_prices(
		std::string_view(itemdata_json.c_str(), itemdata_json.size()),
		std::string_view(compact_json.c_str(), compact_json.size()),
		logger);
}

}
