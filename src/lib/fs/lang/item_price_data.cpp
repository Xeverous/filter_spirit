#include "fs/lang/item_price_data.hpp"
#include "fs/log/logger.hpp"

namespace fs::lang
{

void item_price_data::log_info(log::logger& logger) const
{
	logger.info() << "item price data:\n"
		"\tdivination cards    : " << static_cast<int>(divination_cards.size()) << "\n"
		"\tprophecies          : " << static_cast<int>(prophecies.size()) << "\n"
		"\tessences            : " << static_cast<int>(essences.size()) << "\n"
		"\tleaguestones        : " << static_cast<int>(leaguestones.size()) << "\n"
		"\tpieces              : " << static_cast<int>(pieces.size()) << "\n"
		"\tnets                : " << static_cast<int>(nets.size()) << "\n"
		"\tvials               : " << static_cast<int>(vials.size()) << "\n"
		"\tfossils             : " << static_cast<int>(fossils.size()) << "\n"
		"\tresonators          : " << static_cast<int>(resonators.size()) << "\n"
		"\tscarabs             : " << static_cast<int>(scarabs.size()) << "\n"
		"\tbases (no influence): " << static_cast<int>(bases_without_influence.size()) << "\n"
		"\tbases shaper        : " << static_cast<int>(bases_shaper.size()) << "\n"
		"\tbases elder         : " << static_cast<int>(bases_elder.size());
}

}
