#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>

#include <boost/preprocessor/repeat.hpp>

#include <future>
#include <utility>

namespace
{

constexpr auto target_name = "poe.ninja/api";

}

namespace fs::network::poe_ninja
{

api_item_price_data
download_item_price_data(
	const std::string& league_name,
	const download_settings& settings,
	download_info* info,
	log::logger& logger)
{
	std::string league_encoded = url_encode(league_name);

	#define CURRENCY_OVERVIEW_LINK(type) "https://poe.ninja/api/data/currencyoverview" "?type=" #type "&league="
	#define ITEM_OVERVIEW_LINK(type)     "https://poe.ninja/api/data/itemoverview"     "?type=" #type "&league="

	std::vector<std::string> urls = {
		CURRENCY_OVERVIEW_LINK(Currency)    + league_encoded,
		CURRENCY_OVERVIEW_LINK(Fragment)    + league_encoded,
		ITEM_OVERVIEW_LINK(Oil)             + league_encoded,
		ITEM_OVERVIEW_LINK(Incubator)       + league_encoded,
		ITEM_OVERVIEW_LINK(Scarab)          + league_encoded,
		ITEM_OVERVIEW_LINK(Fossil)          + league_encoded,
		ITEM_OVERVIEW_LINK(Resonator)       + league_encoded,
		ITEM_OVERVIEW_LINK(Essence)         + league_encoded,
		ITEM_OVERVIEW_LINK(DivinationCard)  + league_encoded,
		ITEM_OVERVIEW_LINK(Prophecy)        + league_encoded,
		ITEM_OVERVIEW_LINK(SkillGem)        + league_encoded,
		ITEM_OVERVIEW_LINK(BaseType)        + league_encoded,
		ITEM_OVERVIEW_LINK(HelmetEnchant)   + league_encoded,
		ITEM_OVERVIEW_LINK(UniqueMap)       + league_encoded,
		ITEM_OVERVIEW_LINK(Map)             + league_encoded,
		ITEM_OVERVIEW_LINK(UniqueJewel)     + league_encoded,
		ITEM_OVERVIEW_LINK(UniqueFlask)     + league_encoded,
		ITEM_OVERVIEW_LINK(UniqueWeapon)    + league_encoded,
		ITEM_OVERVIEW_LINK(UniqueArmour)    + league_encoded,
		ITEM_OVERVIEW_LINK(UniqueAccessory) + league_encoded,
		ITEM_OVERVIEW_LINK(Beast)           + league_encoded,
	};

	#undef CURRENCY_OVERVIEW_LINK
	#undef ITEM_OVERVIEW_LINK

	download_result result = download(target_name, urls, settings, info, logger);
	// use preprocessor library to auto-repeat some boilerplate
	// z = n + 1 (n is 0-based, z is 1-based), we ignore z
	// d (data) is not needed, we ignore it
	#define MOVE_RESULT_N(z, n, d) std::move(result.results[n].data),
	return api_item_price_data {
		BOOST_PP_REPEAT(21, MOVE_RESULT_N,)
	};
	#undef MOVE_RESULT_N
}

}
