#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/network/url_encode.hpp>
#include <fs/network/async_download.hpp>
#include <fs/log/logger.hpp>

#include <boost/preprocessor/repeat.hpp>

#include <future>
#include <utility>

namespace
{

constexpr auto host = "poe.ninja";

}

namespace fs::network::poe_ninja
{

std::future<api_item_price_data> async_download_item_price_data(std::string league_name, log::logger& logger)
{
	std::string league_encoded = url_encode(league_name);

	#define CURRENCY_OVERVIEW_LINK(type) "/api/data/currencyoverview" "?type=" #type "&league="
	#define ITEM_OVERVIEW_LINK(type)     "/api/data/itemoverview"     "?type=" #type "&league="

	std::vector<std::string> targets = {
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

	auto response_handler = [league = std::move(league_name)](std::vector<boost::beast::http::response<boost::beast::http::string_body>> responses) {
		if (responses.size() != 21u) {
			throw std::logic_error("logic error: downloaded a different "
				"number of files from poe.ninja: expected 21 but got " + std::to_string(responses.size()));
		}

		// z = n + 1, ignore it
		// data is ignored
		#define MOVE_BODY_N(z, n, data) std::move(responses[n]).body(),
		return api_item_price_data {
			BOOST_PP_REPEAT(21, MOVE_BODY_N,)
		};
		#undef MOVE_BODY_N
	};

	return async_download(host, std::move(targets), response_handler, logger);
}

}
