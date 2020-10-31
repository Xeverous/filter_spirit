#include <fs/network/poe_watch/parse_data.hpp>
#include <fs/network/exceptions.hpp>
#include <fs/utility/dump_json.hpp>
#include <fs/utility/algorithm.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/utility/visitor.hpp>
#include <fs/utility/assert.hpp>
#include <fs/log/logger.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <iterator>
#include <optional>
#include <variant>
#include <type_traits>
#include <utility>

namespace
{

using namespace fs;

BETTER_ENUM(frame_type, int,
	// NOTE: values must be exactly in this order - they must match
	// https://pathofexile.gamepedia.com/Public_stash_tab_API#frameType
	normal,
	magic,
	rare,
	unique,
	gem,
	currency,
	divination_card,
	quest_item,
	prophecy,
	relic,
	// should be last
	unknown)

namespace categories // avoids name conflicts with types in lang namespace
{
	// https://api.poe.watch/categories
	// names of these are expected to match poe.watch's JSON strings
	// we use _to_string on these enums

	BETTER_ENUM(accessory_type, int, amulet, belt, ring, unknown = -1)
	struct accessory
	{
		accessory_type type;
	};

	BETTER_ENUM(armour_type, int, boots, chest, gloves, helmet, quiver, shield, unknown = -1)
	struct armour
	{
		armour_type type;
	};

	struct divination_card {};

	BETTER_ENUM(currency_type, int,
		currency,
		essence,
		piece,
		fossil,
		resonator,
		vial,
		net,
		incubator,
		splinter,
		oil,
		catalyst,
		influence,
		unknown = -1)
	struct currency
	{
		currency_type type;
	};

	BETTER_ENUM(enchantment_type, int, boots, gloves, helmet, unknown = -1)
	struct enchantment
	{
		enchantment_type type;
	};

	struct flask {};

	BETTER_ENUM(gem_type, int, skill, support, vaal, unknown = -1)
	struct gem
	{
		gem_type type;
		int level;
		int quality;
		bool is_corrupted;
	};

	struct jewel {};

	BETTER_ENUM(map_type, int, map, fragment, unique, scarab, watchstone, unknown = -1)
	struct map
	{
		map_type type;
		std::optional<int> series;
		std::optional<int> tier;
	};

	struct prophecy {};

	BETTER_ENUM(weapon_type, int,
		bow,
		claw,
		dagger,
		oneaxe,
		onemace,
		onesword,
		rod,
		sceptre,
		staff,
		twoaxe,
		twomace,
		twosword,
		wand,
		runedagger,
		warstaff,
		unknown = -1)
	struct weapon
	{
		weapon_type type;
	};

	BETTER_ENUM(base_type, int,
		amulet,
		belt,
		ring,
		boots,
		chest,
		gloves,
		helmet,
		quiver,
		shield,
		bow,
		claw,
		dagger,
		oneaxe,
		onemace,
		onesword,
		sceptre,
		staff,
		twoaxe,
		twomace,
		twosword,
		wand,

		// these 2 are no longer reported but keep them for back-compat with past downloads
		jewel,
		rod,

		unknown = -1)
	struct base
	{
		base_type type;
		int ilvl;
		bool is_shaper;
		bool is_elder;
		bool is_crusader;
		bool is_redeemer;
		bool is_hunter;
		bool is_warlord;
	};

	BETTER_ENUM(beast_type, int, beast, /* (Metamorph) */ sample, unknown = -1)
	struct beast
	{
		beast_type type;
	};
} // namespace categories

using item_category_variant = std::variant<
	categories::accessory,
	categories::armour,
	categories::divination_card,
	categories::currency,
	categories::enchantment,
	categories::flask,
	categories::gem,
	categories::jewel,
	categories::map,
	categories::prophecy,
	categories::weapon,
	categories::base,
	categories::beast>;

struct item
{
	std::size_t id = 0;
	std::string name;                     // item main name, eg "Tabula Rasa"
	std::optional<std::string> base_type; // item base type, eg "Simple Robe", null for non-wearable items
	frame_type frame;                  // frame displayed in game UI
	std::optional<int> max_stack_size; // only for stackable items
	std::optional<int> links;
	bool has_unique_variations; // (eg Vessel of Vinktar has 4 variations)
	item_category_variant category; // combined category and group
};

log::message_stream& operator<<(log::message_stream& stream, const item& item)
{
	stream << "item:\n"
		"ID              : " << item.id << "\n"
		"name            : " << item.name << "\n"
		"base type       : " << item.base_type.value_or("(none)") << "\n"
		"frame           : " << (item.frame == +frame_type::unknown ? "(unknown)" : item.frame._to_string()) << "\n";

	const auto log_optional = [&stream](const char* name, std::optional<int> opt) {
		stream << name;
		if (opt)
			stream << *opt;
		else
			stream << "(none)";
		stream << "\n";
	};

	log_optional("max stack size  : ", item.max_stack_size);

	log_optional("links           : ", item.links);

	stream << "has variations  : " << (item.has_unique_variations ? "true" : "false") << "\n";

	stream << "category        : ";

	std::visit(utility::visitor{
		[&](categories::accessory)       { stream << "accessory\n"; },
		[&](categories::armour)          { stream << "armour\n"; },
		[&](categories::divination_card) { stream << "divination card\n"; },
		[&](categories::currency)        { stream << "currency\n"; },
		[&](categories::enchantment)     { stream << "enchantment\n"; },
		[&](categories::flask)           { stream << "flask\n"; },
		[&](categories::gem gem) {
			stream << "gem:\n"
				"\tlevel    : " << gem.level << "\n"
				"\tquality  : " << gem.quality << "\n"
				"\tcorrupted: " << (gem.is_corrupted ? "true" : "false") << "\n";
		},
		[&](categories::jewel)           { stream << "jewel\n"; },
		[&](categories::map map) {
			stream << "map\n\ttype  : " << map.type._name() << "\n";
			log_optional("\ttier  : ", map.tier);
			log_optional("\tseries: ", map.series);
		},
		[&](categories::prophecy)        { stream << "prophecy\n"; },
		[&](categories::weapon)          { stream << "weapon\n"; },
		[&](categories::base base) {
			stream << "base:\n"
				"\ttype      : " << base.type._name() << "\n"
				"\titem level: " << base.ilvl << "\n"
				"\tinfluence : " <<
					(base.is_shaper   ? "Shaper "   : "" ) <<
					(base.is_elder    ? "Elder "    : "" ) <<
					(base.is_crusader ? "Crusader " : "" ) <<
					(base.is_redeemer ? "Redeemer " : "" ) <<
					(base.is_hunter   ? "Hunter "   : "" ) <<
					(base.is_warlord  ? "Warlord "  : "" ) << "\n";
		},
		[&](categories::beast b) {
			switch (b.type) {
				case categories::beast_type::beast:
					stream << "beast";
					break;
				case categories::beast_type::sample:
					stream << "metamorph sample";
					break;
				case categories::beast_type::unknown:
					stream << "(unknown)";
					break;
			}
			stream << '\n';
		}
	}, item.category);

	return stream;
}

// same behaviour as JavaScript code on poe.watch
bool is_low_confidence(int daily, int current)
{
	return daily < 10 || current < 10;
}

// example query: https://api.poe.watch/compact?league=Standard
// vector index is item ID
[[nodiscard]] std::vector<std::optional<lang::market::price_data>>
parse_compact(std::string_view compact_json, log::logger& logger)
{
	nlohmann::json json = nlohmann::json::parse(compact_json);

	if (!json.is_array())
		throw network::json_parse_error("compact JSON must be an array but it is not");

	/*
	 * poe.watch has a relative database. Some of API endpoints can be matched together.
	 * - /itemdata reports an array of {item properties + item ID}
	 * - /compact reports an array of {price stats + item ID}
	 *
	 * This function parses compact JSON so we make an array of price_data objects.
	 * IDs in the database are auto-incremented by 1 for each new item so instead
	 * of using a map<price_data, id> we have a vector<price_data> where item ID is
	 * the index. This gives the fastest possible lookup. We store items in optional
	 * in case there are some gaps due to removed items.
	 */
	std::vector<std::optional<lang::market::price_data>> item_prices;
	// expect about 30 000 items, round to power of 2 for optimal allocator call
	item_prices.resize(32768);
	std::size_t max_id = 0;
	for (const auto& item : json) {
		const auto id = item.at("id").get<std::size_t>();

		// Prevent out-of-memory problems in case of API ID changes or bugs.
		// Assuming sizeof(typename item_prices::value_type) == 24, we limit
		// max used memory by a single compact query to ~400 MB.
		if (id > (1 << 24)) {
			logger.warning() << "A price data entry has abnormally big ID: "
				<< id << ", skipping this entry\n";
			continue;
		}

		if (id >= item_prices.size()) // C++20: [[unlikely]]
			item_prices.resize(id * 2);

		if (id > max_id)
			max_id = id;

		if (item_prices[id].has_value()) { // C++20: [[unlikely]]
			logger.warning() << "A price data entry with duplicated ID has been found, ID = "
				<< id << ", skipping this entry\n";
			continue;
		}

		item_prices[id] = lang::market::price_data{
			item.at("mean").get<double>(),
			is_low_confidence(
				item.at("daily").get<int>(),
				item.at("current").get<int>())};
	}

	item_prices.resize(max_id);
	return item_prices;
}

frame_type parse_item_frame(const nlohmann::json& json)
{
	if (!json.is_number())
		return frame_type::unknown;

	const auto val = json.get<int>();
	const auto maybe_enum = frame_type::_from_index_nothrow(val);

	if (maybe_enum)
		return *maybe_enum;
	else
		return frame_type::unknown;
}

template <typename EnumType>
EnumType json_to_enum(const nlohmann::json& json)
{
	// static_assert(std::is_enum_v<EnumType>, "type must be a better enum"); // BETTER_ENUM(e, ...) macro does not actually create enum e
	static_assert(sizeof(decltype(EnumType::unknown)) != 0, "EnumType must have an \"unknown\" member");

	if (!json.is_string())
		return EnumType::unknown;

	const auto& str = json.get_ref<const nlohmann::json::string_t&>();
	const auto maybe_result = EnumType::_from_string_nothrow(str.c_str());
	if (maybe_result)
		return *maybe_result;
	else
		return EnumType::unknown;
}

template <typename T, typename Arg> [[nodiscard]]
std::optional<T> get_optional_subelement(const nlohmann::json& obj_json, Arg&& arg)
{
	FS_ASSERT(obj_json.is_object());

	if (auto it = obj_json.find(std::forward<Arg>(arg)); it != obj_json.end()) {
		auto& item = *it;

		if (!item.is_null())
			return item.template get<T>();
	}

	return std::nullopt;
}

item_category_variant parse_item_category(const nlohmann::json& entry)
{
	const nlohmann::json& category = entry.at("category");
	const nlohmann::json& group = entry.at("group");

	if (!category.is_string())
		throw network::json_parse_error("item category should be a string but it is not");

	const auto& category_str = category.get_ref<const nlohmann::json::string_t&>();

	if (category_str == "accessory") {
		return categories::accessory{json_to_enum<categories::accessory_type>(group)};
	}
	if (category_str == "armour") {
		return categories::armour{json_to_enum<categories::armour_type>(group)};
	}
	if (category_str == "card") {
		return categories::divination_card{};
	}
	if (category_str == "currency") {
		return categories::currency{json_to_enum<categories::currency_type>(group)};
	}
	if (category_str == "enchantment") {
		return categories::enchantment{json_to_enum<categories::enchantment_type>(group)};
	}
	if (category_str == "flask") {
		return categories::flask{};
	}
	if (category_str == "gem") {
		const auto gem_lvl      = entry.at("gemLevel")  .get<int>();
		const auto gem_quality  = entry.at("gemQuality").get<int>();
		const auto is_corrupted = entry.at("gemIsCorrupted").get<bool>();

		return categories::gem{json_to_enum<categories::gem_type>(group), gem_lvl, gem_quality, is_corrupted};
	}
	if (category_str == "jewel") {
		return categories::jewel{};
	}
	if (category_str == "map") {
		const auto series = get_optional_subelement<int>(entry, "mapSeries");
		const auto tier   = get_optional_subelement<int>(entry, "mapTier");

		return categories::map{json_to_enum<categories::map_type>(group), series, tier};
	}
	if (category_str == "prophecy") {
		return categories::prophecy{};
	}
	if (category_str == "weapon") {
		return categories::weapon{json_to_enum<categories::weapon_type>(group)};
	}
	if (category_str == "base") {
		const auto& influences = entry.at("influences");
		if (!influences.is_array())
			throw network::json_parse_error("item influences should be an array but it is not");

		const bool is_shaper   = influences.find("shaper")   != influences.end();
		const bool is_elder    = influences.find("elder")    != influences.end();
		const bool is_crusader = influences.find("crusader") != influences.end();
		const bool is_redeemer = influences.find("redeemer") != influences.end();
		const bool is_hunter   = influences.find("hunter")   != influences.end();
		const bool is_warlord  = influences.find("warlord")  != influences.end();

		const auto ilvl = entry.at("baseItemLevel").get<int>();

		return categories::base{
			json_to_enum<categories::base_type>(group),
			ilvl,
			is_shaper,
			is_elder,
			is_crusader,
			is_redeemer,
			is_hunter,
			is_warlord
		};
	}
	if (category_str == "beast") {
		return categories::beast{json_to_enum<categories::beast_type>(group)};
	}

	throw network::json_parse_error("item has unrecognized category: " + category_str);
}

[[nodiscard]]
item parse_item(const nlohmann::json& item_json)
{
	FS_ASSERT(item_json.is_object());

	return item{
		item_json.at("id").get<std::size_t>(),
		item_json.at("name").get<std::string>(),
		get_optional_subelement<std::string>(item_json, "type"),
		parse_item_frame(item_json.at("frame")),
		get_optional_subelement<int>(item_json, "stackSize"),
		get_optional_subelement<int>(item_json, "linkCount"),
		item_json.find("variation") != item_json.end(),
		parse_item_category(item_json)};
}

// https://api.poe.watch/itemdata
[[nodiscard]] std::vector<item>
parse_itemdata(std::string_view itemdata_json, log::logger& logger)
{
	nlohmann::json json = nlohmann::json::parse(itemdata_json);

	if (!json.is_array()) // C++20: [[unlikely]]
		throw network::json_parse_error("itemdata JSON must be an array but it is not");

	std::vector<item> items;
	items.reserve(32768); // expect about 30 000 items
	for (const auto& item_entry : json) {
		const auto log_exception = [&](const auto& e) {
			logger.warning() << "failed to parse item entry in itemdata JSON: " << e.what()
				<< ", skipping the following item: " << utility::dump_json(item_entry) << '\n';
		};

		try {
			items.push_back(parse_item(item_entry));
		}
		catch (const network::json_parse_error& e) {
			log_exception(e);
		}
		catch (const nlohmann::json::exception& e) {
			log_exception(e);
		}
	}

	return items;
}

} // namespace

namespace fs::network::poe_watch
{

// https://api.poe.watch/leagues
std::vector<lang::league> parse_league_info(std::string_view league_json)
{
	nlohmann::json json = nlohmann::json::parse(league_json);

	if (!json.is_array()) // C++20: [[unlikely]]
		throw network::json_parse_error("league JSON must be an array but it is not");

	struct league_with_id
	{
		int id;
		lang::league league;
	};

	std::vector<league_with_id> leagues;
	leagues.reserve(json.size());

	for (const auto& item : json) {
		leagues.push_back(league_with_id{
			item.at("id").get<int>(),
			lang::league{
				item.at("name").get<std::string>(),
				item.at("hardcore").get<bool>()
			}
		});
	}

	// sort leagues by ID - this will improve consistency in the output
	std::sort(
		leagues.begin(),
		leagues.end(),
		[](const league_with_id& left, const league_with_id& right) {
			return left.id < right.id;
		});

	std::vector<lang::league> result;
	result.reserve(leagues.size());

	for (league_with_id& lwi : leagues)
		result.push_back(std::move(lwi.league));

	return result;
}

lang::market::item_price_data
parse_item_price_data(
	const api_item_price_data& ipd,
	log::logger& logger)
{
	logger.info() << "parsing item prices\n";
	std::vector<std::optional<lang::market::price_data>> item_prices = parse_compact(ipd.compact_json, logger);
	if (item_prices.empty())
		throw network::json_parse_error("parsed empty list of item prices");

	logger.info() << "parsing item data\n";
	std::vector<item> itemdata = parse_itemdata(ipd.itemdata_json, logger);
	if (itemdata.empty())
		throw network::json_parse_error("parsed empty list of item data");

	logger.info() << "item entries: " << itemdata.size() << '\n';

	lang::market::item_price_data result;
	for (item& itm : itemdata) {
		/*
		 * Ignore items which do not have price information.
		 *
		 * poe.watch has a global database for all items which is shared across leagues.
		 * Some items are not obtainable in all leagues so this is normal. Additionally,
		 * the owner confirmed that the lack of price infromation can be a result of
		 * no confidence about given item offers.
		 */
		if (itm.id >= item_prices.size() || !item_prices[itm.id].has_value()) {
			continue;
		}

		const lang::market::price_data& price_data = *item_prices[itm.id];

		// ignore items with low confidence
		if (price_data.is_low_confidence)
			continue;

		// ignore beasts - they do not drop
		// note: this needs to be before base type checking - beasts do not have base type
		if (std::holds_alternative<categories::beast>(itm.category))
			continue;

		// skip relic items - they do not drop in an ordinary way
		if (itm.frame == +frame_type::relic)
			continue;

		using lang::market::elementary_item;

		if (itm.frame == +frame_type::unique) {
			if (!itm.base_type) {
				logger.warning() << "A unique item without base type has been found. Ignored item: " << itm;
				continue;
			}

			if ((*itm.base_type).empty()) {
				logger.warning() << "A unique item with empty string base type has been found. Ignored item: " << itm;
				continue;
			}

			// skip uniques which have links - we care about sole unique item value, not linked items
			if (itm.links)
				continue;

			// skip uniques which do not drop (eg fated items) - this will reduce ambiguity and
			// not pollute the filter with items we would not care for
			if (lang::market::is_undroppable_unique(itm.name))
				continue;

			if (std::holds_alternative<categories::armour>(itm.category)
				|| std::holds_alternative<categories::weapon>(itm.category)
				|| std::holds_alternative<categories::accessory>(itm.category))
			{
				result.unique_eq.add_item(
					std::move(*itm.base_type),
					elementary_item{price_data, std::move(itm.name)});
				continue;
			}
			if (std::holds_alternative<categories::jewel>(itm.category)) {
				result.unique_jewels.add_item(
					std::move(*itm.base_type),
					elementary_item{price_data, std::move(itm.name)});
				continue;
			}
			if (std::holds_alternative<categories::flask>(itm.category)) {
				result.unique_flasks.add_item(
					std::move(*itm.base_type),
					elementary_item{price_data, std::move(itm.name)});
				continue;
			}
			if (std::holds_alternative<categories::map>(itm.category)) {
				result.unique_maps.add_item(
					std::move(*itm.base_type),
					elementary_item{price_data, std::move(itm.name)});
				continue;
			}

			// poe.ninja reports Harbinger pieces as unique items - make poe.watch consistent with it
			// TODO test how Harbinger pieces are recognized by the item filter (are they of unique rarity? are they currency?)
			if (std::holds_alternative<categories::currency>(itm.category)) {
				const auto& currency = std::get<categories::currency>(itm.category);

				if (currency.type == +categories::currency_type::piece) {
					result.unique_eq.add_item(
						std::move(*itm.base_type),
						elementary_item{price_data, std::move(itm.name)});
					continue;
				}
			}

			logger.warning() << "A unique item has been skipped because it's category was not recognized: " << itm;
		}
		else if (std::holds_alternative<categories::currency>(itm.category)) {
			const auto& curr = std::get<categories::currency>(itm.category);

			if (curr.type == +categories::currency_type::essence) {
				result.essences.push_back(elementary_item{price_data, std::move(itm.name)});
			}
			else if (curr.type == +categories::currency_type::fossil) {
				result.fossils.push_back(elementary_item{price_data, std::move(itm.name)});
			}
			else if (curr.type == +categories::currency_type::resonator) {
				result.resonators.push_back(elementary_item{price_data, std::move(itm.name)});
			}
			else if (curr.type == +categories::currency_type::incubator) {
				result.incubators.push_back(elementary_item{price_data, std::move(itm.name)});
			}
			else if (curr.type == +categories::currency_type::oil) {
				result.oils.push_back(elementary_item{price_data, std::move(itm.name)});
			}
			if (curr.type == +categories::currency_type::catalyst) {
				result.catalysts.push_back(elementary_item{price_data, std::move(itm.name)});
			}

			// we do not care about other currency items
			continue;
		}
		else if (std::holds_alternative<categories::divination_card>(itm.category)) {
			if (!itm.max_stack_size) {
				logger.warning() << "A divination card without stack size specified has been found: " << itm
					<< "Assuming for safety the card might have a stack size of 1, which so far was the case with this bug.\n";
			}

			result.divination_cards.push_back(lang::market::divination_card{
				elementary_item{price_data, std::move(itm.name)},
				itm.max_stack_size.value_or(1)});
			continue;
		}
		else if (std::holds_alternative<categories::prophecy>(itm.category)) {
			result.prophecies.push_back(elementary_item{price_data, std::move(itm.name)});
			continue;
		}
		else if (std::holds_alternative<categories::map>(itm.category)) {
			const auto& map = std::get<categories::map>(itm.category);

			if (map.type == +categories::map_type::scarab)
				result.scarabs.push_back(elementary_item{price_data, std::move(itm.name)});

			// We do not care about other map items.
			// We do actually want to make use of Watchstone price data but
			// unfortunately all unique watchstones have "Ivory Watchstone" base.
			continue;
		}
		else if (std::holds_alternative<categories::base>(itm.category)) {
			const auto& base = std::get<categories::base>(itm.category);

			result.bases.push_back(lang::market::base{
				elementary_item{price_data, std::move(itm.name)},
				base.ilvl,
				base.is_shaper,
				base.is_elder,
				base.is_crusader,
				base.is_redeemer,
				base.is_hunter,
				base.is_warlord});
			continue;
		}
		else if (std::holds_alternative<categories::gem>(itm.category)) {
			const auto& gem = std::get<categories::gem>(itm.category);

			result.gems.push_back(lang::market::gem{
				elementary_item{price_data, std::move(itm.name)},
				gem.level,
				gem.quality,
				gem.is_corrupted});
			continue;
		}
		else if (std::holds_alternative<categories::enchantment>(itm.category)) {
			const auto& ench = std::get<categories::enchantment>(itm.category);

			if (ench.type == +categories::enchantment_type::helmet) {
				result.helmet_enchants.push_back(elementary_item{price_data, std::move(itm.name)});
			}

			// we do not care about other enchants
			continue;
		}

		logger.warning() << "An item did not match any category or group, skipping: " << itm <<
			"Please report this even if the item does not make sense for use in filters as this"
			" problem might actually be hiding some more complex bug.";
	}

	return result;
}

}
