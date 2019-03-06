#pragma once

#include "itemdata/types.hpp"

#include <string>
#include <iosfwd>

namespace fs::compiler
{

[[nodiscard]]
bool process_input(const std::string& input, const itemdata::item_price_data& items, std::ostream& error_stream);

}
