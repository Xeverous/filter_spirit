#include <fs/lang/market/item_price_data.hpp>
#include <fs/network/poe_ninja/api_data.hpp>
#include <fs/network/poe_watch/api_data.hpp>
#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/poe_watch/parse_data.hpp>
#include <fs/network/exceptions.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/file.hpp>
#include <fs/utility/algorithm.hpp>
#include <fs/utility/dump_json.hpp>
#include <fs/utility/string_helpers.hpp>

#include <nlohmann/json.hpp>

#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
		// these can actually drop in a map with Corrupting Tempest or some sextant mods
		// "Blood of Corruption",
		// "Ancient Waystones"
		// "Atziri's Reign",
		// "Blood Sacrifice",
		// "Brittle Barrier",
		// "Chill of Corruption",
		// "Combustibles",
		// "Corrupted Energy",
		// "Fevered Mind",
		// "Fragility",
		// "Hungry Abyss",
		// "Mutated Growth",
		// "Pacifism",
		// "Powerlessness",
		// "Sacrificial Harvest",
		// "Self-Flagellation",
		// "Vaal Sentencing",
		// "Weight of Sin",
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

using namespace fs;
using namespace fs::lang;
using namespace fs::lang::market;

void compare_item_price_data(
	const item_price_data& lhs,
	const item_price_data& rhs,
	log::message_stream& stream)
{
	// divination cards
	{
		const auto compare_div_card_eq =
			[](const divination_card& lhs, const divination_card& rhs) {
				return lhs.stack_size == rhs.stack_size && lhs.name == rhs.name;
			};

		const auto compare_div_card_lt =
			[](const divination_card& lhs, const divination_card& rhs) {
				return std::tie(lhs.name, lhs.stack_size) < std::tie(rhs.name, rhs.stack_size);
			};

		boost::format lhs_fmter("%-47s %2s |\n");
		const auto report_lhs = [&](const divination_card& c) { stream << (lhs_fmter % c.name % c.stack_size).str(); };

		boost::format rhs_fmter("                                                   | %-47s %2s\n");
		const auto report_rhs = [&](const divination_card& c) { stream << (rhs_fmter % c.name % c.stack_size).str(); };

		stream << "divination cards (name + stack size): " << lhs.divination_cards.size()
			<< " vs " << rhs.divination_cards.size() << '\n';
		utility::diff_report(
			lhs.divination_cards.begin(), lhs.divination_cards.end(),
			rhs.divination_cards.begin(), rhs.divination_cards.end(),
			compare_div_card_eq, compare_div_card_lt,
			report_lhs, report_rhs);
	}

	const auto compare_elementary_items_eq =
		[](const elementary_item& lhs, const elementary_item& rhs) {
			return lhs.name == rhs.name;
		};

	const auto compare_elementary_items_lt =
		[](const elementary_item& lhs, const elementary_item& rhs) {
			return lhs.name < rhs.name;
		};

	boost::format lhs_fmter("%-50s |\n");
	const auto report_lhs = [&](const elementary_item& e) { stream << (lhs_fmter % e.name).str(); };

	boost::format rhs_fmter("                                                   | %s\n");
	const auto report_rhs = [&](const elementary_item& e) { stream << (rhs_fmter % e.name).str(); };

	const auto run_compare_elementary =
		[&](const std::vector<elementary_item>& lhs, const std::vector<elementary_item>& rhs, const char* desc) {
			stream << desc << ": " << lhs.size() << " vs " << rhs.size() << '\n';
			utility::diff_report(
				lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
				compare_elementary_items_eq, compare_elementary_items_lt,
				report_lhs, report_rhs);
		};

	run_compare_elementary(lhs.oils, rhs.oils, "oils");
	run_compare_elementary(lhs.incubators, rhs.incubators, "incubators");
	run_compare_elementary(lhs.essences, rhs.essences, "essences");
	run_compare_elementary(lhs.fossils, rhs.fossils, "fossils");
	run_compare_elementary(lhs.prophecies, rhs.prophecies, "prophecies");
	run_compare_elementary(lhs.resonators, rhs.resonators, "resonators");
	run_compare_elementary(lhs.scarabs, rhs.scarabs, "scarabs");

	// currently poe.watch and poe.ninja represent enchants in different forms (#% vs 30%)
	// run_compare_elementary(lhs.helmet_enchants, rhs.helmet_enchants, "helmet enchants");

	// much more complex to report (2+ properties)
	// run_compare_elementary(lhs.gems, rhs.gems, "gems");
	// run_compare_elementary(lhs.bases, rhs.bases, "bases");
}

} // namespace

namespace fs::lang::market
{

bool is_undroppable_unique(std::string_view name) noexcept
{
	return std::binary_search(undroppable_uniques.begin(), undroppable_uniques.end(), name);
}

bool item_price_data::load_and_parse(
	const item_price_metadata& metadata,
	const std::filesystem::path& directory_path,
	log::logger& logger)
{
	try {
		if (metadata.data_source == lang::data_source_type::poe_ninja) {
			network::poe_ninja::api_item_price_data api_data;

			if (!api_data.load(directory_path, logger)) {
				logger.error() << "Failed to load item price data.\n";
				return false;
			}

			*this = network::poe_ninja::parse_item_price_data(api_data, logger);
			return true;
		}
		else if (metadata.data_source == lang::data_source_type::poe_watch) {
			network::poe_watch::api_item_price_data api_data;

			if (!api_data.load(directory_path, logger)) {
				logger.error() << "Failed to load item price data.\n";
				return false;
			}

			*this = network::poe_watch::parse_item_price_data(api_data, logger);
			return true;
		}
		else {
			logger.error() << "Unknown data source.\n";
			return false;
		}
	}
	catch (const network::json_parse_error& e) {
		logger.error() << "Failed to parse JSON file: " << e.what() << '\n';
	}
	catch (const nlohmann::json::exception& e) {
		// no need to add more text, nlohmann exceptions are verbose
		logger.error() << e.what() << '\n';
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

log::message_stream& operator<<(log::message_stream& stream, const item_price_data& ipd)
{
	return stream << "Item price data:\n"
		"\tdivination cards: " << ipd.divination_cards.size() << "\n"
		"\tcatalysts       : " << ipd.catalysts.size() << "\n"
		"\toils            : " << ipd.oils.size() << "\n"
		"\tincubators      : " << ipd.incubators.size() << "\n"
		"\tessences        : " << ipd.essences.size() << "\n"
		"\tfossils         : " << ipd.fossils.size() << "\n"
		"\tprophecies      : " << ipd.prophecies.size() << "\n"
		"\tresonators      : " << ipd.resonators.size() << "\n"
		"\tscarabs         : " << ipd.scarabs.size() << "\n"
		// not supported (yet)
		// "\thelmet enchants : " << ipd.helmet_enchants.size() << "\n"
		"\tgems            : " << ipd.gems.size() << "\n"
		"\tbases           : " << ipd.bases.size() << "\n"
		"\tunique equipment (unambiguous): " << ipd.unique_eq.unambiguous.size() << "\n"
		"\tunique equipment (ambiguous)  : " << ipd.unique_eq.ambiguous.size() << "\n"
		"\tunique flasks (unambiguous)   : " << ipd.unique_flasks.unambiguous.size() << "\n"
		"\tunique flasks (ambiguous)     : " << ipd.unique_flasks.ambiguous.size() << "\n"
		"\tunique jewels (unambiguous)   : " << ipd.unique_jewels.unambiguous.size() << "\n"
		"\tunique jewels (ambiguous)     : " << ipd.unique_jewels.ambiguous.size() << "\n"
		"\tunique maps (unambiguous)     : " << ipd.unique_maps.unambiguous.size() << "\n"
		"\tunique maps (ambiguous)       : " << ipd.unique_maps.ambiguous.size() << "\n";
}

log::message_stream& operator<<(log::message_stream& stream, const item_price_metadata& ipm)
{
	return stream << "Item price metadata:\n"
		"\titem price data downloaded: " << utility::ptime_to_pretty_string(ipm.download_date) << "\n"
		"\titem price data from      : " << std::string(lang::to_string(ipm.data_source)) << "\n"
		"\titem price data for league: " << ipm.league_name << "\n";
}

log::message_stream& operator<<(log::message_stream& stream, const item_price_report& ipr)
{
	return stream << ipr.metadata << ipr.data;
}

void item_price_data::sort()
{
	const auto compare_by_name_asc =
		[](const elementary_item& lhs, const elementary_item& rhs) {
			return lhs.name < rhs.name;
		};

	std::sort(divination_cards.begin(), divination_cards.end(), compare_by_name_asc);
	std::sort(catalysts.begin(),        catalysts.end(),        compare_by_name_asc);
	std::sort(oils.begin(),             oils.end(),             compare_by_name_asc);
	std::sort(incubators.begin(),       incubators.end(),       compare_by_name_asc);
	std::sort(essences.begin(),         essences.end(),         compare_by_name_asc);
	std::sort(fossils.begin(),          fossils.end(),          compare_by_name_asc);
	std::sort(prophecies.begin(),       prophecies.end(),       compare_by_name_asc);
	std::sort(resonators.begin(),       resonators.end(),       compare_by_name_asc);
	std::sort(scarabs.begin(),          scarabs.end(),          compare_by_name_asc);
	std::sort(helmet_enchants.begin(),  helmet_enchants.end(),  compare_by_name_asc);
	std::sort(gems.begin(),             gems.end(),             compare_by_name_asc);
	std::sort(bases.begin(),            bases.end(),            compare_by_name_asc);
}

void compare_item_price_reports(
	const item_price_report& lhs,
	const item_price_report& rhs,
	log::logger& logger)
{
	auto stream = logger.info();
	stream << "Left price data:\n" << lhs.metadata << "Right price data:\n" << rhs.metadata;
	compare_item_price_data(lhs.data, rhs.data, stream);
}

constexpr auto filename_metadata = "metadata.json";

constexpr auto field_league_name = "league_name";
constexpr auto field_data_source = "data_source";
constexpr auto field_download_date = "download_date";

nlohmann::json to_json(const item_price_metadata& metadata)
{
	return {
		{field_league_name, metadata.league_name},
		{field_data_source, to_string(metadata.data_source)},
		{field_download_date, boost::posix_time::to_iso_string(metadata.download_date)}
	};
}

std::optional<item_price_metadata> from_json(const nlohmann::json& json, log::logger& logger)
{
	item_price_metadata data;
	data.league_name = json.at(field_league_name).get<std::string>();

	if (
		std::optional<data_source_type> data_source = lang::from_string(
			json.at(field_data_source).get_ref<const std::string&>());
		data_source)
	{
		data.data_source = *data_source;
	}
	else {
		logger.error() << "Failed to parse data source from metadata file.\n";
		return std::nullopt;
	}

	data.download_date = boost::posix_time::from_iso_string(
		json.at(field_download_date).get_ref<const std::string&>());
	if (data.download_date.is_special()) {
		logger.error() << "Invalid date in metadata file.\n";
		return std::nullopt;
	}

	return data;
}

bool item_price_metadata::save(const std::filesystem::path& directory, log::logger& logger) const
{
	if (!utility::create_directories(directory, logger))
		return false;

	const auto path = directory / filename_metadata;
	std::error_code ec = utility::save_file(path, utility::dump_json(to_json(*this)));

	if (ec) {
		logger.error() << "Failed to save " << path.generic_string() << ": " << ec.message() << ".\n";
		return false;
	}

	return true;
}

bool item_price_metadata::load(const std::filesystem::path& directory, log::logger& logger)
{
	std::error_code ec;
	const auto path = directory / filename_metadata;
	std::string file_content = utility::load_file(path, ec);

	if (ec) {
		logger.error() << "Failed to load " << path.generic_string() << ": " << ec.message() << ".\n";
		return false;
	}

	try {
		auto json = nlohmann::json::parse(file_content);
		// move from a separate instance to implement strong exception guuarantee
		std::optional<item_price_metadata> metadata = from_json(json, logger);

		if (!metadata) {
			logger.error() << "Failed to parse metadata JSON file.\n";
			return false;
		}

		*this = std::move(*metadata);
		return true;
	}
	catch (const std::exception& e) {
		logger.error() << "When reading " << path.generic_string() << ": " << e.what() << ".\n";
		return false;
	}
}

std::optional<item_price_report>
load_item_price_report(
	const std::filesystem::path& directory,
	log::logger& logger)
{
	item_price_report report;
	if (!report.metadata.load(directory, logger)) {
		logger.error() << "Failed to load item price metadata.\n";
		return std::nullopt;
	}

	if (!report.data.load_and_parse(report.metadata, directory, logger)) {
		logger.error() << "Failed to load item price data.\n";
		return std::nullopt;
	}

	return report;
}

}
