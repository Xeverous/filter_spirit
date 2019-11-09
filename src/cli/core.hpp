#pragma once

#include <fs/log/logger_fwd.hpp>
#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_price_metadata.hpp>
#include <fs/lang/data_source_type.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

void list_leagues(fs::log::logger& logger);

[[nodiscard]] std::optional<fs::lang::item_price_data>
download_item_price_data(
	const std::string& league_name,
	fs::lang::data_source_type data_source,
	fs::log::logger& logger);

[[nodiscard]] bool
generate_item_filter(
	const fs::lang::item_price_data& item_price_data,
	const fs::lang::item_price_metadata& item_price_metadata,
	const boost::filesystem::path& source_filepath,
	const boost::filesystem::path& output_filepath,
	bool print_ast,
	fs::log::logger& logger);
