#include "itemdata/types.hpp"
#include "log/logger.hpp"

namespace fs::itemdata
{

void item_price_data::log_info(logger& logger) const
{
	logger.info() << "item price data:\n"
		"\tdivination cards    : " << divination_cards.size() << "\n"
		"\tprophecies          : " << prophecies.size() << "\n"
		"\tHarbinger pieces    : " << harbinger_pieces.size() << "\n"
		"\tbases (no influence): " << bases_without_influence.size() << "\n"
		"\tbases shaper        : " << bases_shaper.size() << "\n"
		"\tbases elder         : " << bases_elder.size() << "\n";
}

}
