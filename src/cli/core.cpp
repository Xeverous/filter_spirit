#include "core.hpp"

#include <fs/generator/generate_filter.hpp>
#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/poe_watch/download_data.hpp>
#include <fs/network/poe_watch/parse_data.hpp>
#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

using namespace fs;

void list_leagues(log::logger& logger)
{
	std::future<network::poe_watch::api_league_data> leagues_future = network::poe_watch::async_download_leagues(logger);
	const auto league_data = leagues_future.get();
	const std::vector<lang::league> leagues = network::poe_watch::parse_league_info(league_data.leagues);

	logger.begin_info_message();
	logger.add("available leagues:\n");

	// print leaue.name because that's the name that the API expects
	// printing different name style would confuse users - these names are quered
	// to be later to the API as league name
	for (const lang::league& league : leagues)
		logger << league.name << "\n";

	logger.end_message();
}

std::optional<lang::item_price_data>
download_item_price_data(
	const std::string& league_name,
	lang::data_source_type data_source,
	log::logger& logger)
{
	try {
		if (data_source == lang::data_source_type::poe_ninja) {
			std::future<network::poe_ninja::api_item_price_data> ipd_future =
				network::poe_ninja::async_download_item_price_data(league_name, logger);
			const auto ipd = ipd_future.get();
			return network::poe_ninja::parse_item_price_data(ipd, logger);
		}
		else if (data_source == lang::data_source_type::poe_watch) {
			std::future<network::poe_watch::api_item_price_data> ipd_future =
				network::poe_watch::async_download_item_price_data(league_name, logger);
			const auto ipd = ipd_future.get();
			return network::poe_watch::parse_item_price_data(ipd, logger);
		}
		else {
			logger.error() << "unknown data source";
		}
	}
	catch (const std::exception& e) {
		logger.error() << e.what();
	}

	return std::nullopt;
}

bool generate_item_filter(
	const lang::item_price_data& item_price_data,
	const lang::item_price_metadata& item_price_metadata,
	const boost::filesystem::path& source_filepath,
	const boost::filesystem::path& output_filepath,
	bool print_ast,
	log::logger& logger)
{
	std::error_code ec;
	std::string source_file_content = utility::load_file(source_filepath, ec);

	if (ec) {
		logger.error() << "failed to load " << source_filepath.generic_string() << ", " << ec.message();
		return false;
	}

	std::optional<std::string> filter_content = generator::generate_filter(
		source_file_content,
		item_price_data,
		item_price_metadata,
		generator::options{print_ast},
		logger);

	if (!filter_content)
		return false;

	ec = utility::save_file(output_filepath, *filter_content);
	if (ec) {
		logger.error() << "can not save filter content: " << ec.message();
		return false;
	}

	return true;
}
