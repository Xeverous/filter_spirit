#pragma once

#include "fs/lang/item_price_data.hpp"
#include "fs/generator/options.hpp"
#include "fs/generator/metadata.hpp"

#include <string_view>
#include <optional>

namespace fs::log { class logger; }

namespace fs::generator
{

/**
 * @brief end-to-end filter generation function
 *
 * @param input filter template source code
 * @param item_price_data item price data
 * @param options filter generation options
 * @param metadata filter's metadata
 * @param logger logger instance
 * @return filter file content or nothing if error occured
 */
[[nodiscard]]
std::optional<std::string> generate_filter(
	std::string_view input,
	const lang::item_price_data& item_price_data,
	options options,
	metadata metadata,
	log::logger& logger);

// mostly for tests
[[nodiscard]]
std::optional<std::string> generate_filter_without_preamble(
	std::string_view input,
	const lang::item_price_data& item_price_data,
	options options,
	log::logger& logger);

}
