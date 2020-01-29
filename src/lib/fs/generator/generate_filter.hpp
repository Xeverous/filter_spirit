#pragma once

#include <fs/lang/item_price_data.hpp>
#include <fs/generator/options.hpp>
#include <fs/log/logger.hpp>

#include <string_view>
#include <optional>

namespace fs::generator::sf
{

/**
 * @brief end-to-end filter generation function
 *
 * @param input filter template source code
 * @param item_price_info item price data and metadata
 * @param options filter generation options
 * @param logger logger instance
 * @return filter file content or nothing if error occured
 */
[[nodiscard]]
std::optional<std::string> generate_filter(
	std::string_view input,
	const lang::item_price_info& item_price_info,
	options options,
	log::logger& logger);

// mostly for tests
[[nodiscard]]
std::optional<std::string> generate_filter_without_preamble(
	std::string_view input,
	const lang::item_price_data& item_price_data,
	options options,
	log::logger& logger);

}
