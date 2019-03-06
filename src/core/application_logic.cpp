#include "core/application_logic.hpp"
#include "utility/file.hpp"
#include "compiler/compiler.hpp"
#include <iostream>
#include <exception>

namespace fs::core
{

bool application_logic::generate_filter(const std::string& league_name, const std::string& source_filepath, const std::string& output_filepath)
{
	std::optional<std::string> source_file_content = fs::utility::load_file(source_filepath);

	if (!source_file_content)
	{
		std::cout << "error: could not load the input file\n";
		return false;
	}

	try {
		std::future<itemdata::item_price_data> ipd_future = storage.async_download_item_price_data(league_name);
		itemdata::item_price_data ipd = ipd_future.get();
		return fs::compiler::process_input(*source_file_content, ipd, std::cout);
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << e.what() << "\n";
		return false;
	}
}

void application_logic::list_leagues()
{
	std::future<std::vector<itemdata::league>> leagues_future = storage.async_download_leagues();
	const auto& leagues = leagues_future.get();
	for (const itemdata::league& league : leagues)
		std::cout << league.display_name << '\n';
}

}
