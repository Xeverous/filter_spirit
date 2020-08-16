#pragma once

#include <fs/log/logger.hpp>
#include <fs/network/download.hpp>
#include <fs/generator/settings.hpp>
#include <fs/lang/market/item_price_data.hpp>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>

void
list_leagues(
	fs::network::download_settings settings,
	fs::log::logger& logger);

[[nodiscard]] std::optional<fs::lang::market::item_price_report>
obtain_item_price_report(
	const boost::optional<std::string>& download_league_name_ninja,
	const boost::optional<std::string>& download_league_name_watch,
	boost::posix_time::time_duration expiration_time,
	fs::network::download_settings settings,
	const boost::optional<std::string>& data_read_dir,
	fs::log::logger& logger);

[[nodiscard]] bool
generate_item_filter(
	const std::optional<fs::lang::market::item_price_report>& report,
	const boost::optional<std::string>& source_filepath,
	const boost::optional<std::string>& output_filepath,
	fs::generator::settings st,
	fs::log::logger& logger);

[[nodiscard]] int // <= exit status
print_item_price_report(
	const std::string& path,
	fs::log::logger& logger);

[[nodiscard]] int // <= exit status
compare_data_saves(
	const std::vector<std::string>& paths,
	fs::log::logger& logger);
