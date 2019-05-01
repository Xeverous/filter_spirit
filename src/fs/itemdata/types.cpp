#include "fs/itemdata/types.hpp"
#include "fs/log/logger.hpp"

namespace fs::itemdata
{

void item_price_data::log_info(log::logger& logger) const
{
	logger.info() << "item price data:\n"
		"\tdivination cards    : " << static_cast<int>(divination_cards.size()) << "\n"
		"\tprophecies          : " << static_cast<int>(prophecies.size()) << "\n"
		"\tHarbinger pieces    : " << static_cast<int>(harbinger_pieces.size()) << "\n"
		"\tbases (no influence): " << static_cast<int>(bases_without_influence.size()) << "\n"
		"\tbases shaper        : " << static_cast<int>(bases_shaper.size()) << "\n"
		"\tbases elder         : " << static_cast<int>(bases_elder.size());
}

}
