#pragma once

#include <fs/log/logger.hpp>
#include <fs/network/download.hpp>
#include <fs/generator/settings.hpp>
#include <fs/lang/item_price_data.hpp>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include <string>

void
list_leagues(
	fs::network::network_settings net_settings,
	fs::log::logger& logger);

[[nodiscard]] std::optional<fs::lang::item_price_info>
obtain_item_price_info(
	const boost::optional<std::string>& download_league_name_ninja,
	const boost::optional<std::string>& download_league_name_watch,
	fs::network::network_settings net_settings,
	const boost::optional<std::string>& data_read_dir,
	const boost::optional<std::string>& data_save_dir,
	fs::log::logger& logger);

[[nodiscard]] bool
generate_item_filter(
	const std::optional<fs::lang::item_price_info>& item_data,
	const boost::optional<std::string>& source_filepath,
	const boost::optional<std::string>& output_filepath,
	fs::generator::settings st,
	fs::log::logger& logger);

[[nodiscard]] int // <= exit status
print_data_save_info(
	const std::string& path,
	fs::log::logger& logger);

[[nodiscard]] int // <= exit status
compare_data_saves(
	const std::vector<std::string>& paths,
	fs::log::logger& logger);
