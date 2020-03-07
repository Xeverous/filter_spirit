#include "core.hpp"

#include <fs/generator/generate_filter.hpp>
#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/poe_watch/download_data.hpp>
#include <fs/network/poe_watch/parse_data.hpp>
#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

using namespace fs;

namespace
{

[[nodiscard]] std::optional<lang::item_price_info>
load_item_price_info(const std::string& path, log::logger& logger)
{
	lang::item_price_info info;
	if (!info.metadata.load(path, logger)) {
		logger.error() << "failed to load item price metadata\n";
		return std::nullopt;
	}

	if (!info.data.load_and_parse(info.metadata, path, logger)) {
		logger.error() << "failed to load item price data\n";
		return std::nullopt;
	}

	return info;
}

template <typename T>
void save_api_data(
	const T& api_data, // T must have save(dir, logger) overload
	const lang::item_price_metadata& metadata,
	const std::string& data_save_dir,
	log::logger& logger)
{
	if (!metadata.save(data_save_dir, logger)) {
		logger.error() << "failed to save item price metadata\n";
		return;
	}

	if (!api_data.save(data_save_dir, logger)) {
		logger.error() << "failed to save item price data\n";
	}

	logger.info() << "item price data successfully saved\n";
}

bool generate_item_filter_impl(
	const lang::item_price_info& item_price_info,
	const boost::filesystem::path& source_filepath,
	const boost::filesystem::path& output_filepath,
	bool print_ast,
	log::logger& logger)
{
	std::optional<std::string> source_file_content = utility::load_file(source_filepath, logger);

	if (!source_file_content)
		return false;

	std::optional<std::string> filter_content = generator::sf::generate_filter(
		*source_file_content,
		item_price_info,
		generator::options{print_ast},
		logger);

	if (!filter_content)
		return false;

	if (utility::save_file(output_filepath, *filter_content, logger)) {
		logger.info() << "item filter successfully saved as " << output_filepath.generic_string() << '\n';
		return true;
	}
	else {
		return false;
	}
}

} // namespace

void list_leagues(network::network_settings net_settings, log::logger& logger)
{
	const auto league_data = network::poe_watch::async_download_leagues(net_settings, logger).get();
	const std::vector<lang::league> leagues = network::poe_watch::parse_league_info(league_data.leagues);

	auto stream = logger.info();
	stream << "available leagues:\n";

	// print leaue.name because that's the name that the API expects
	// printing different name style would confuse users - these names are quered
	// to be later to the API as league name
	for (const lang::league& league : leagues) {
		if (league.is_active)
			stream << league.name << '\n';
		else if (league.is_upcoming)
			stream << league.name << " (upcoming)\n";
	}
}

std::optional<lang::item_price_info>
obtain_item_price_info(
	const boost::optional<std::string>& download_league_name_ninja,
	const boost::optional<std::string>& download_league_name_watch,
	network::network_settings net_settings,
	const boost::optional<std::string>& data_read_dir,
	const boost::optional<std::string>& data_save_dir,
	fs::log::logger& logger)
{
	if ((download_league_name_watch && data_read_dir)
		|| (download_league_name_ninja && data_read_dir)
		|| (download_league_name_ninja && download_league_name_watch))
	{
		logger.error() << "more than 1 data obtaining option specified\n";
		return std::nullopt;
	}

	if (data_read_dir && data_save_dir) {
		logger.error() << "reading and saving can not be specified at the same time\n";
		return std::nullopt;
	}

	if (data_read_dir) {
		return load_item_price_info(*data_read_dir, logger);
	}

	lang::item_price_info info;
	if (download_league_name_ninja) {
		auto api_data = network::poe_ninja::async_download_item_price_data(
			*download_league_name_ninja, net_settings, logger).get();

		info.metadata.data_source = lang::data_source_type::poe_ninja;
		info.metadata.league_name = *download_league_name_ninja;
		info.metadata.download_date = boost::posix_time::microsec_clock::universal_time();

		if (data_save_dir)
			save_api_data(api_data, info.metadata, *data_save_dir, logger);

		info.data = network::poe_ninja::parse_item_price_data(api_data, logger);
	}
	else if (download_league_name_watch) {
		auto api_data = network::poe_watch::async_download_item_price_data(
			*download_league_name_watch, net_settings, logger).get();

		info.metadata.data_source = lang::data_source_type::poe_watch;
		info.metadata.league_name = *download_league_name_watch;
		info.metadata.download_date = boost::posix_time::microsec_clock::universal_time();

		if (data_save_dir)
			save_api_data(api_data, info.metadata, *data_save_dir, logger);

		info.data = network::poe_watch::parse_item_price_data(api_data, logger);
	}

	return info;
}

bool generate_item_filter(
	const std::optional<lang::item_price_info>& item_price_info,
	const boost::optional<std::string>& input_path,
	const boost::optional<std::string>& output_path,
	bool print_ast,
	fs::log::logger& logger)
{
	if (!item_price_info) {
		logger.error() << "no item price data, giving up on filter generation\n";
		return false;
	}

	if (!input_path) {
		logger.error() << "no input path given\n";
		return false;
	}

	if (!output_path) {
		logger.error() << "no output path given\n";
		return false;
	}

	return generate_item_filter_impl(*item_price_info, *input_path, *output_path, print_ast, logger);
}

int print_data_save_info(
	const std::string& path,
	fs::log::logger& logger)
{
	std::optional<lang::item_price_info> data = load_item_price_info(path, logger);
	if (!data) {
		return EXIT_FAILURE;
	}

	logger.info() << "" << *data;

	return EXIT_SUCCESS;
}

int compare_data_saves(
	const std::vector<std::string>& paths,
	fs::log::logger& logger)
{
	if (paths.size() != 2u) {
		logger.error() << "for comparing data saves exactly 2 paths should be given\n";
		return EXIT_FAILURE;
	}

	std::optional<lang::item_price_info> data_lhs = load_item_price_info(paths[0], logger);
	if (!data_lhs) {
		return EXIT_FAILURE;
	}

	std::optional<lang::item_price_info> data_rhs = load_item_price_info(paths[1], logger);
	if (!data_lhs) {
		return EXIT_FAILURE;
	}

	(*data_lhs).data.sort();
	(*data_rhs).data.sort();

	lang::compare_item_price_info(*data_lhs, *data_rhs, logger);

	return EXIT_SUCCESS;
}
