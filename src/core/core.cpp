#include "core/core.hpp"
#include "utility/file.hpp"
#include "network/poe_watch_api.hpp"
#include "compiler/compiler.hpp"
#include "itemdata/parse_json.hpp"

#include <iostream>
#include <fstream>

namespace fs::core
{

bool generate_item_filter(
	const itemdata::item_price_data& item_price_data,
	const std::string& source_filepath,
	const std::string& output_filepath)
{
	std::optional<std::string> source_file_content = fs::utility::load_file(source_filepath);

	if (!source_file_content)
	{
		std::cout << "error: can not open input file\n"; // TODO replace with logger
		return false;
	}

	std::optional<std::string> filter_content = fs::compiler::process_input(*source_file_content, item_price_data, std::cout); // TODO pass logger instance instead of std::cout
	if (!filter_content)
		return false;

	std::ofstream output_file(output_filepath);
	if (!output_file.good())
	{
		std::cout << "error: can not open output file\n";
		return false;
	}

	output_file << *filter_content;
	return true;
}

void list_leagues()
{
	std::future<std::vector<itemdata::league>> leagues_future = network::poe_watch_api::async_download_leagues();
	const auto& leagues = leagues_future.get();
	for (const itemdata::league& league : leagues)
		std::cout << league.display_name << '\n';
}

itemdata::item_price_data download_item_price_data(const std::string& league_name)
{
	try
	{
		std::future<itemdata::item_price_data> ipd_future = network::poe_watch_api::async_download_item_price_data(league_name);
		return ipd_future.get();
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << e.what() << "\n"; // TODO replace with logger
		throw;
	}
}

std::optional<itemdata::item_price_data> load_item_price_data(const std::string& directory_path)
{
	std::optional<std::string> compact = utility::load_file(directory_path + "/compact.json");

	if (!compact)
	{
		std::cout << "error: can not open compact.json\n";
		return std::nullopt;
	}

	std::optional<std::string> itemdata = utility::load_file(directory_path + "/itemdata.json");

	if (!compact)
	{
		std::cout << "error: can not open itemdata.json\n";
		return std::nullopt;
	}

	const std::string& compact_json = *compact;
	const std::string& itemdata_json = *itemdata;
	return itemdata::parse_item_prices(
		std::string_view(itemdata_json.c_str(), itemdata_json.size()),
		std::string_view(compact_json.c_str(), compact_json.size()));
}

}
