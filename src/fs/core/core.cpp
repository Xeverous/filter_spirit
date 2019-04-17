#include "fs/core/core.hpp"
#include "fs/utility/file.hpp"
#include "fs/network/poe_watch_api.hpp"
#include "fs/compiler/compiler.hpp"
#include "fs/itemdata/parse_json.hpp"

#include <fstream>

namespace fs::core
{

bool generate_item_filter(
	const itemdata::item_price_data& item_price_data,
	const std::string& source_filepath,
	const std::string& output_filepath,
	bool print_ast,
	logger& logger)
{
	std::optional<std::string> source_file_content = utility::load_file(source_filepath);

	if (!source_file_content)
	{
		logger.error() << "can not open input file";
		return false;
	}

	std::optional<std::string> filter_content = compiler::process_input(*source_file_content, item_price_data, print_ast, logger);
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

void list_leagues(logger& logger)
{
	std::future<std::vector<itemdata::league>> leagues_future = network::poe_watch_api::async_download_leagues();
	const auto& leagues = leagues_future.get();

	logger.begin_info_message();
	logger.add("available leagues:\n");

	for (const itemdata::league& league : leagues)
		logger << league.display_name << "\n";

	logger.end_message();
}

itemdata::item_price_data download_item_price_data(const std::string& league_name, logger& logger)
{
	try
	{
		std::future<itemdata::item_price_data> ipd_future = network::poe_watch_api::async_download_item_price_data(league_name, logger);
		return ipd_future.get();
	}
	catch (const std::exception& e)
	{
		logger.error() << e.what();
		throw;
	}
}

std::optional<itemdata::item_price_data> load_item_price_data(const std::string& directory_path, logger& logger)
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
	return itemdata::parse_item_prices(
		std::string_view(itemdata_json.c_str(), itemdata_json.size()),
		std::string_view(compact_json.c_str(), compact_json.size()),
		logger);
}

}
