#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/lang/item_price_data.hpp>

#include <string>
#include <vector>

namespace fs::generator
{

[[nodiscard]]
std::string to_raw_filter(const lang::item_filter& filter);

void prepend_metadata(const lang::item_price_metadata& metadata, std::string& raw_filter);

}
