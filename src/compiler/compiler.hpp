#pragma once

#include "itemdata/types.hpp"

#include <string>
#include <iosfwd> // TODO logger, remove this
#include <optional>

namespace fs::compiler
{

/**
 * @brief end-to-end filter generation function
 *
 * @param input filter template source code
 * @param item_price_data item price data
 * @param error_stream logger instance
 * @return filter file content or nothing if error occured
 */
[[nodiscard]]
std::optional<std::string> process_input(const std::string& input, const itemdata::item_price_data& item_price_data, std::ostream& error_stream);

}
