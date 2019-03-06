#pragma once

#include "network/poe_watch_api.hpp"

namespace fs::itemdata
{

class data_storage
{
public:
	std::future<std::vector<itemdata::league>> async_download_leaues();

private:
	network::poe_watch_api api;
};

}
