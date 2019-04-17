/**
 * @file core application functionality wrappers
 */

#pragma once

#include "fs/itemdata/types.hpp"
#include "fs/log/logger.hpp"

#include <string>
#include <optional>

namespace fs::core
{

bool generate_item_filter(
	const itemdata::item_price_data& item_price_data,
	const std::string& source_filepath,
	const std::string& output_filepath,
	bool print_ast,
	logger& logger);

void list_leagues(logger& logger);

/**
 * @brief download item price data from poe.watch's API
 * @param league_name valid league name that is currently curring
 * @return item price data
 *
 * @details you can obtain currently available leagues by calling @ref list_leagues()
 *
 * this function throws upon network and parsing failures
 */
itemdata::item_price_data download_item_price_data(const std::string& league_name, logger& logger);
/**
 * @brief load item price data from saved JSON files
 * @param directory_path path where compact.json and itemdata.json are present
 * @return item price data or nothing in case of error
 *
 * @details you can download JSON files from poe.watch
 */
std::optional<itemdata::item_price_data> load_item_price_data(const std::string& directory_path, logger& logger);

}
