#include "itemdata/data_storage.hpp"

namespace fs::itemdata
{

std::future<std::vector<itemdata::league>> data_storage::async_download_leaues()
{
	return api.async_download_leagues();
}

}
