#pragma once

#include <fs/network/ggg/api_data.hpp>
#include <fs/lang/league.hpp>

#include <vector>

namespace fs::network::ggg
{

[[nodiscard]] std::vector<lang::league>
parse_league_info(
	api_league_data leagues);

}
