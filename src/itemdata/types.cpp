#include "itemdata/types.hpp"
#include "log/logger.hpp"

namespace fs::itemdata
{

void item_price_data::log_info(logger& logger) const
{
	logger.info() << "itemdata:\n"
		<< "divination cards: " << divination_cards.size() << "\n"
		<< "prophecies      : " << prophecies.size() << "\n"
		<< "bases no inf    : " << bases_without_influence.size() << "\n"
		<< "bases shaper    : " << bases_shaper.size() << "\n"
		<< "bases elder     : " << bases_elder.size() << "\n";
}

}
