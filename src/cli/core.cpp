#include "core.hpp"

#include <fs/generator/common.hpp>
#include <fs/network/item_price_report.hpp>
#include <fs/network/ggg/download_data.hpp>
#include <fs/network/ggg/parse_data.hpp>
#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

#include <filesystem>
#include <utility>

using namespace fs;

namespace
{

bool generate_item_filter_impl(
	const lang::market::item_price_report& report,
	const std::filesystem::path& source_filepath,
	const std::filesystem::path& output_filepath,
	fs::generator::settings st,
	log::logger& logger)
{
	std::optional<std::string> source_file_content = utility::load_file(source_filepath, logger);

	if (!source_file_content)
		return false;

	std::optional<std::string> filter_content = generator::parse_compile_generate_spirit_filter(
		*source_file_content, report, st, logger);

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

void list_leagues(network::download_settings settings, log::logger& logger)
{
	const auto league_data = network::ggg::async_download_leagues(std::move(settings), nullptr, logger).get();
	const std::vector<lang::league> leagues = network::ggg::parse_league_info(league_data);

	auto stream = logger.info();
	stream << "available leagues:\n";

	for (const lang::league& league : leagues)
		stream << league.name << '\n';
}

std::optional<lang::market::item_price_report>
obtain_item_price_report(
	const boost::optional<std::string>& download_league_name_ninja,
	const boost::optional<std::string>& download_league_name_watch,
	boost::posix_time::time_duration expiration_time,
	network::download_settings settings,
	const boost::optional<std::string>& data_read_dir,
	fs::log::logger& logger)
{
	if ((download_league_name_watch && data_read_dir)
		|| (download_league_name_ninja && data_read_dir)
		|| (download_league_name_ninja && download_league_name_watch))
	{
		logger.error() << "more than 1 data obtaining option specified\n";
		return std::nullopt;
	}

	if (data_read_dir) {
		return lang::market::load_item_price_report(*data_read_dir, logger);
	}

	network::item_price_report_cache cache;
	cache.load_cache_file_from_disk(logger);
	if (download_league_name_ninja) {
		return cache.async_get_report(
			std::move(*download_league_name_ninja),
			lang::data_source_type::poe_ninja,
			expiration_time,
			std::move(settings),
			nullptr,
			logger).get();
	}
	else if (download_league_name_watch) {
		return cache.async_get_report(
			std::move(*download_league_name_watch),
			lang::data_source_type::poe_watch,
			expiration_time,
			std::move(settings),
			nullptr,
			logger).get();
	}

	logger.error() << "no option specified how to obtain item price data\n";
	return std::nullopt;
}

bool generate_item_filter(
	const std::optional<lang::market::item_price_report>& report,
	const boost::optional<std::string>& input_path,
	const boost::optional<std::string>& output_path,
	fs::generator::settings st,
	fs::log::logger& logger)
{
	if (!report) {
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

	return generate_item_filter_impl(*report, *input_path, *output_path, st, logger);
}

int print_item_price_report(
	const std::string& path,
	fs::log::logger& logger)
{
	std::optional<lang::market::item_price_report> report = lang::market::load_item_price_report(path, logger);
	if (!report) {
		return EXIT_FAILURE;
	}

	logger.info() << "" << *report;

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

	std::optional<lang::market::item_price_report> report_lhs = lang::market::load_item_price_report(paths[0], logger);
	if (!report_lhs) {
		return EXIT_FAILURE;
	}

	std::optional<lang::market::item_price_report> report_rhs = lang::market::load_item_price_report(paths[1], logger);
	if (!report_rhs) {
		return EXIT_FAILURE;
	}

	(*report_lhs).data.sort();
	(*report_rhs).data.sort();

	lang::market::compare_item_price_reports(*report_lhs, *report_rhs, logger);

	return EXIT_SUCCESS;
}
