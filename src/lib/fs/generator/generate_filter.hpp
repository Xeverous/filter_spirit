#pragma once

#include <fs/lang/item_price_data.hpp>
#include <fs/generator/settings.hpp>

#include <optional>
#include <string_view>

namespace fs::generator::sf
{

/**
 * @brief end-to-end filter generation function
 *
 * @param input filter template source code
 * @param item_price_report item price data and metadata
 * @param st filter generation settings
 * @param logger logger instance
 * @return filter file content or nothing if error occured
 */
[[nodiscard]]
std::optional<std::string> generate_filter(
	std::string_view input,
	const lang::item_price_report& report,
	settings st,
	log::logger& logger);

// mostly for tests
[[nodiscard]]
std::optional<std::string> generate_filter_without_preamble(
	std::string_view input,
	const lang::item_price_data& item_price_data,
	settings st,
	log::logger& logger);

}
