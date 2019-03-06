#include "itemdata/data_storage.hpp"

namespace fs::itemdata
{

std::future<std::vector<itemdata::league>> data_storage::async_download_leagues()
{
	return api.async_download_leagues();
}

std::future<itemdata::item_price_data> data_storage::async_download_item_price_data(std::string league_name)
{
	return api.async_download_item_prices(league_name);
}

}
