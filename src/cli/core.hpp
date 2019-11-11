#pragma once

#include <fs/log/logger_fwd.hpp>
#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_price_metadata.hpp>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include <string>

void list_leagues(fs::log::logger& logger);

struct item_data
{
	fs::lang::item_price_data item_price_data;
	fs::lang::item_price_metadata item_price_metadata;
};

[[nodiscard]] std::optional<item_data>
obtain_item_data(
	const boost::optional<std::string>& download_league_name_ninja,
	const boost::optional<std::string>& download_league_name_watch,
	const boost::optional<std::string>& data_read_dir,
	const boost::optional<std::string>& data_save_dir,
	fs::log::logger& logger);

[[nodiscard]] bool
generate_item_filter(
	const std::optional<item_data>& item_data,
	const boost::optional<std::string>& source_filepath,
	const boost::optional<std::string>& output_filepath,
	bool print_ast,
	fs::log::logger& logger);
