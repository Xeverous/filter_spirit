#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/lang/market/item_price_data.hpp>

#include <string>
#include <string_view>

namespace fs::generator
{

[[nodiscard]]
std::string to_string(const lang::item_filter& filter);

void prepend_metadata(const lang::market::item_price_metadata& metadata, std::string& raw_filter);

}
