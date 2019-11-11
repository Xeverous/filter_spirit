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

template <typename T>
void save_data(
	const boost::optional<std::string>& data_save_dir,
	const T& api_data, // T must have save(dir, logger) overload
	const lang::item_price_metadata& metadata,
	log::logger& logger)
{
	if (!data_save_dir)
		return;

	auto& dir = *data_save_dir;

	if (!metadata.save(dir, logger)) {
		logger.error() << "failed to save item price metadata";
		return;
	}

	if (!api_data.save(dir, logger)) {
		logger.error() << "failed to save item price data";
	}

	logger.info() << "item price data successfully saved";
}

bool generate_item_filter_impl(
	const item_data& item_data,
	const boost::filesystem::path& source_filepath,
	const boost::filesystem::path& output_filepath,
	bool print_ast,
	log::logger& logger)
{
	std::optional<std::string> source_file_content = utility::load_file(source_filepath, logger);

	if (!source_file_content)
		return false;

	std::optional<std::string> filter_content = generator::generate_filter(
		*source_file_content,
		item_data.item_price_data,
		item_data.item_price_metadata,
		generator::options{print_ast},
		logger);

	if (!filter_content)
		return false;

	return utility::save_file(output_filepath, *filter_content, logger);
}

} // namespace

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

std::optional<item_data>
obtain_item_data(
	const boost::optional<std::string>& download_league_name_ninja,
	const boost::optional<std::string>& download_league_name_watch,
	const boost::optional<std::string>& data_read_dir,
	const boost::optional<std::string>& data_save_dir,
	fs::log::logger& logger)
{
	if ((download_league_name_watch && data_read_dir)
		|| (download_league_name_ninja && data_read_dir)
		|| (download_league_name_ninja && download_league_name_watch))
	{
		logger.error() << "more than 1 data obtaining option specified";
		return std::nullopt;
	}

	if (data_read_dir && data_save_dir) {
		logger.error() << "reading and saving can not be specified at the same time";
		return std::nullopt;
	}

	item_data data;
	if (download_league_name_ninja) {
		auto api_data = network::poe_ninja::async_download_item_price_data(*download_league_name_ninja, logger).get();

		data.item_price_metadata.data_source = lang::data_source_type::poe_ninja;
		data.item_price_metadata.league_name = *download_league_name_ninja;
		data.item_price_metadata.download_date = boost::posix_time::microsec_clock::universal_time();

		save_data(data_save_dir, api_data, data.item_price_metadata, logger);

		data.item_price_data = network::poe_ninja::parse_item_price_data(api_data, logger);
	}
	else if (download_league_name_watch) {
		auto api_data = network::poe_watch::async_download_item_price_data(*download_league_name_watch, logger).get();

		data.item_price_metadata.data_source = lang::data_source_type::poe_watch;
		data.item_price_metadata.league_name = *download_league_name_watch;
		data.item_price_metadata.download_date = boost::posix_time::microsec_clock::universal_time();

		if (data_save_dir)
			save_data(*data_save_dir, api_data, data.item_price_metadata, logger);

		data.item_price_data = network::poe_watch::parse_item_price_data(api_data, logger);
	}
	else if (data_read_dir) {
		if (!data.item_price_metadata.load(*data_read_dir, logger)) {
			logger.error() << "failed to load item price metadata";
			return std::nullopt;
		}

		if (!data.item_price_data.load_and_parse(data.item_price_metadata, *data_read_dir, logger)) {
			logger.error() << "failed to load item price data";
			return std::nullopt;
		}
	}

	return data;
}

[[nodiscard]] bool
generate_item_filter(
	const std::optional<item_data>& item_data,
	const boost::optional<std::string>& input_path,
	const boost::optional<std::string>& output_path,
	bool print_ast,
	fs::log::logger& logger)
{
	if (!item_data) {
		logger.error() << "no item price data, giving up on filter generation";
		return false;
	}

	if (!input_path) {
		logger.error() << "no input path given";
		return false;
	}

	if (!output_path) {
		logger.error() << "no output path given";
		return false;
	}

	return generate_item_filter_impl(*item_data, *input_path, *output_path, print_ast, logger);
}
