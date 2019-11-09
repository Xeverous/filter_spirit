#pragma once

#include <fs/network/poe_ninja/api_data.hpp>
#include <fs/log/logger_fwd.hpp>

#include <future>
#include <string>

namespace fs::network::poe_ninja
{

[[nodiscard]]
std::future<api_item_price_data> async_download_item_price_data(std::string league_name, log::logger& logger);

}
