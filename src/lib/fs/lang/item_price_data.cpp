#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_price_metadata.hpp>
#include <fs/network/poe_ninja/api_data.hpp>
#include <fs/network/poe_watch/api_data.hpp>
#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/poe_watch/parse_data.hpp>
#include <fs/network/exceptions.hpp>
#include <fs/log/logger.hpp>

#include <nlohmann/json.hpp>

#include <array>
#include <algorithm>
#include <cstring>
#include <utility>

namespace
{

// C++20: use constexpr std::sort
auto make_undroppable_uniques()
{
	auto undroppable_uniques = std::array{
		// vendor recipe only
		"The Anima Stone",
		"Arborix",
		"Duskdawn",
		"The Goddess Scorned",
		"The Goddess Unleashed",
		// "Hyrri's Bite", // this can actually drop
		"Kingmaker",
		"Loreweave",
		"Magna Eclipsis",
		"The Retch",
		"Star of Wraeclast",
		"The Taming",
		"The Vinktar Square",

		// fated uniques (Prophecy upgrade only)
		"Amplification Rod",
		"Asenath's Chant",
		"Atziri's Reflection",
		"Cameria's Avarice",
		"Chaber Cairn",
		"Corona Solaris",
		"Cragfall",
		"Crystal Vault",
		"Death's Opus",
		"Deidbellow",
		"Doedre's Malevolence",
		"Doomfletch's Prism",
		"Dreadbeak",
		"Dreadsurge",
		"Duskblight",
		"Ezomyte Hold",
		"Fox's Fortune",
		"Frostferno",
		"Geofri's Devotion",
		"Geofri's Legacy",
		"Greedtrap",
		"Hrimburn",
		"Hrimnor's Dirge",
		"Hyrri's Demise",
		"Kaltensoul",
		"Kaom's Way",
		"Karui Charge",
		"Malachai's Awakening",
		"Martyr's Crown",
		"Mirebough",
		"Ngamahu Tiki",
		"Panquetzaliztli",
		"Queen's Escape",
		"Realm Ender",
		"Sanguine Gambol",
		"Shavronne's Gambit",
		"Silverbough",
		"Sunspite",
		"The Cauteriser",
		"The Dancing Duo",
		"The Effigon",
		"The Gryphon",
		"The Iron Fortress",
		"The Nomad",
		"The Oak",
		"The Signal Fire",
		"The Stormwall",
		"The Tactician",
		"The Tempest",
		"Thirst for Horrors",
		"Timetwist",
		"Voidheart",
		"Wall of Brambles",
		"Whakatutuki o Matua",
		"Wildwrap",
		"Windshriek",
		"Winterweave",

		// blessed uniques (Breach Blessing upgrade only)
		"Xoph's Nurture",
		"The Formless Inferno",
		"Xoph's Blood",
		"Tulfall",
		"The Perfect Form",
		"The Pandemonius",
		"Hand of Wisdom and Action",
		"Esh's Visage",
		"Choir of the Storm",
		"Uul-Netol's Embrace",
		"The Red Trail",
		"The Surrender",
		"United in Dream",
		"Skin of the Lords",
		"Presence of Chayula",
		"The Red Nightmare",
		"The Green Nightmare",
		"The Blue Nightmare",

		// Incursion temple upgrade only
		"Transcendent Flesh",
		"Transcendent Mind",
		"Transcendent Spirit",
		"Soul Ripper",
		"Slavedriver's Hand",
		"Coward's Legacy",
		"Omeyocan",
		"Fate of the Vaal",
		"Mask of the Stitched Demon",
		"Apep's Supremacy",
		"Zerphi's Heart",
		"Shadowstitch",

		// Harbinger uniques (always drop as pieces)
		"The Flow Untethered",
		"The Fracturing Spinner",
		"The Tempest's Binding",
		"The Rippling Thoughts",
		"The Enmity Divine",
		"The Unshattered Will",

		// corruption only
		"Blood of Corruption",
		"Ancient Waystones"
		"Atziri's Reign",
		"Blood Sacrifice",
		"Brittle Barrier",
		"Chill of Corruption",
		"Combustibles",
		"Corrupted Energy",
		"Fevered Mind",
		"Fragility",
		"Hungry Abyss",
		"Mutated Growth",
		"Pacifism",
		"Powerlessness",
		"Sacrificial Harvest",
		"Self-Flagellation",
		"Vaal Sentencing",
		"Weight of Sin",
	};

	std::sort(
		undroppable_uniques.begin(),
		undroppable_uniques.end(),
		[](const char* lhs, const char* rhs) {
			return std::strcmp(lhs, rhs) < 0;
		}
	);

	return undroppable_uniques;
}

const auto undroppable_uniques = make_undroppable_uniques();

}

namespace fs::lang
{

bool is_undroppable_unique(std::string_view name) noexcept
{
	return std::binary_search(undroppable_uniques.begin(), undroppable_uniques.end(), name);
}

bool item_price_data::load_and_parse(
	const item_price_metadata& metadata,
	const std::string& directory_path,
	log::logger& logger)
{
	try {
		if (metadata.data_source == lang::data_source_type::poe_ninja) {
			network::poe_ninja::api_item_price_data api_data;
			const std::error_code ec = api_data.load(directory_path);

			if (ec) {
				logger.error() << "failed to load item price data: " << ec.message();
				return false;
			}

			*this = network::poe_ninja::parse_item_price_data(api_data, logger);
			return true;
		}
		else if (metadata.data_source == lang::data_source_type::poe_watch) {
			network::poe_watch::api_item_price_data api_data;
			const std::error_code ec = api_data.load(directory_path);

			if (ec) {
				logger.error() << "failed to load item price data: " << ec.message();
				return false;
			}

			*this = network::poe_watch::parse_item_price_data(api_data, logger);
			return true;
		}
		else {
			logger.error() << "unknown data source";
			return false;
		}
	}
	catch (const network::json_parse_error& e) {
		logger.warning() << "failed to parse JSON file: " << e.what();
	}
	catch (const nlohmann::json::exception& e) {
		logger.warning() << e.what(); // no need to add more text, nlohmann exceptions are verbose
	}

	return false;
}

void unique_item_price_data::add_item(std::string base_type, elementary_item item)
{
	if (auto it = ambiguous.find(base_type); it != ambiguous.end()) {
		// there is already a unique item with such base type name - add another
		it->second.push_back(std::move(item));
		return;
	}

	if (auto it = unambiguous.find(base_type); it != unambiguous.end()) {
		// we have found a new item with the same base type name
		// move the old one to ambiguous items and add current one there too
		auto& vec = ambiguous[std::move(base_type)];
		vec.push_back(std::move(unambiguous.extract(it).mapped()));
		vec.push_back(std::move(item));
		return;
	}

	// no conflicts found - add the item to unambiguous uniques
	unambiguous.emplace(std::move(base_type), std::move(item));
}

log::logger_wrapper& operator<<(log::logger_wrapper& logger, const item_price_data& ipd)
{
	return logger << "item price data:\n"
		"\tdivination cards: " << static_cast<int>(ipd.divination_cards.size()) << "\n"
		"\toils            : " << static_cast<int>(ipd.oils.size()) << "\n"
		"\tincubators      : " << static_cast<int>(ipd.incubators.size()) << "\n"
		"\tessences        : " << static_cast<int>(ipd.essences.size()) << "\n"
		"\tfossils         : " << static_cast<int>(ipd.fossils.size()) << "\n"
		"\tprophecies      : " << static_cast<int>(ipd.prophecies.size()) << "\n"
		"\tresonators      : " << static_cast<int>(ipd.resonators.size()) << "\n"
		"\tscarabs         : " << static_cast<int>(ipd.scarabs.size()) << "\n"
		"\thelmet enchants : " << static_cast<int>(ipd.helmet_enchants.size()) << "\n"
		"\tgems            : " << static_cast<int>(ipd.gems.size()) << "\n"
		"\tbases           : " << static_cast<int>(ipd.bases.size()) << "\n"
		"\tunique equipment (unambiguous): " << static_cast<int>(ipd.unique_eq.unambiguous.size()) << "\n"
		"\tunique equipment (ambiguous)  : " << static_cast<int>(ipd.unique_eq.ambiguous.size()) << "\n"
		"\tunique flasks (unambiguous)   : " << static_cast<int>(ipd.unique_flasks.unambiguous.size()) << "\n"
		"\tunique flasks (ambiguous)     : " << static_cast<int>(ipd.unique_flasks.ambiguous.size()) << "\n"
		"\tunique jewels (unambiguous)   : " << static_cast<int>(ipd.unique_jewels.unambiguous.size()) << "\n"
		"\tunique jewels (ambiguous)     : " << static_cast<int>(ipd.unique_jewels.ambiguous.size()) << "\n"
		"\tunique map (unambiguous)      : " << static_cast<int>(ipd.unique_maps.unambiguous.size()) << "\n"
		"\tunique map (ambiguous)        : " << static_cast<int>(ipd.unique_maps.ambiguous.size()) << "\n";
}

}
