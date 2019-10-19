#include <fs/network/poe_watch/parse_json.hpp>
#include <fs/network/poe_watch/exceptions.hpp>
#include <fs/utility/algorithm.hpp>
#include <fs/utility/better_enum.hpp>
#include <fs/utility/visitor.hpp>
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

	BETTER_ENUM(accessory_type, int, amulet, belt, ring, unknown = -1)
	struct accessory { accessory_type type; };

	BETTER_ENUM(armour_type, int, boots, chest, gloves, helmet, quiver, shield, unknown = -1)
	struct armour { armour_type type; };

	struct divination_card {};

	BETTER_ENUM(currency_type, int, currency, essence, piece, fossil, resonator, vial, net, incubator, splinter, oil, unknown = -1)
	struct currency { currency_type type; };

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
		bool corrupted;
	};

	struct jewel {};

	BETTER_ENUM(map_type, int, map, fragment, unique, scarab, unknown = -1)
	struct map
	{
		map_type type;
		std::optional<int> series;
		std::optional<int> tier;
	};

	struct prophecy {};

	BETTER_ENUM(weapon_type, int, bow, claw, dagger, oneaxe, onemace, onesword, rod, sceptre, staff, twoaxe, twomace, twosword, wand, runedagger, warstaff, unknown = -1)
	struct weapon { weapon_type type; };

	BETTER_ENUM(base_type, int, amulet, belt, ring, boots, chest, gloves, helmet, quiver, shield, jewel, bow, claw, dagger, oneaxe, onemace, onesword, rod, sceptre, staff, twoaxe, twomace, twosword, wand, unknown = -1)
	struct base
	{
		enum class influence_type { none, shaper, elder };

		base_type type;
		influence_type influence;
		int ilvl;
	};

	struct beast {};
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
	void log_info(log::logger& logger) const;

	int id = 0;
	std::string name;                     // item main name, eg "Tabula Rasa"
	std::optional<std::string> base_type; // item base type, eg "Simple Robe", null for non-wearable items
	frame_type frame;                  // frame displayed in game UI
	std::optional<int> max_stack_size; // only for stackable items
	std::optional<int> links;
	bool has_unique_variations; // (eg Vessel of Vinktar has 4 variations)
	item_category_variant category; // combined category and group
};

void item::log_info(log::logger& logger) const
{
	logger.begin_info_message();
	logger << "item:\n"
		"ID              : " << id << "\n"
		"name            : " << name << "\n"
		"base type       : " << base_type.value_or("(none)") << "\n"
		"frame           : " << (frame == +frame_type::unknown ? "(unknown)" : frame._to_string()) << "\n";

	const auto log_optional = [&logger](const char* name, std::optional<int> opt)
	{
		logger << name;
		if (opt)
			logger << *opt;
		else
			logger << "(none)";
		logger << "\n";
	};

	log_optional("max stack size  : ", max_stack_size);

	log_optional("links           : ", links);

	logger << "has variations  : " << (has_unique_variations ? "true" : "false") << "\n";

	logger << "category        : ";

	std::visit(utility::visitor{
		[&](categories::accessory)       { logger << "accessory\n"; },
		[&](categories::armour)          { logger << "armour\n"; },
		[&](categories::divination_card) { logger << "divination card\n"; },
		[&](categories::currency)        { logger << "currency\n"; },
		[&](categories::enchantment)     { logger << "enchantment\n"; },
		[&](categories::flask)           { logger << "flask\n"; },
		[&](categories::gem gem)
		{
			logger << "gem:\n"
				"\tlevel    : " << gem.level << "\n"
				"\tquality  : " << gem.quality << "\n"
				"\tcorrupted: " << (gem.corrupted ? "true" : "false") << "\n";
		},
		[&](categories::jewel)           { logger << "jewel\n"; },
		[&](categories::map map)
		{
			logger << "map\n\ttype  : " << map.type._name() << "\n";
			log_optional("\ttier  : ", map.tier);
			log_optional("\tseries: ", map.series);
		},
		[&](categories::prophecy)        { logger << "prophecy\n"; },
		[&](categories::weapon)          { logger << "weapon\n"; },
		[&](categories::base base)
		{
			logger << "base:\n"
				"\ttype      : " << base.type._name() << "\n"
				"\tinfluence : " << (
					base.influence == categories::base::influence_type::shaper ? "shaper" :
					base.influence == categories::base::influence_type::elder  ? "elder"  : "none") <<
				"\titem level: " << base.ilvl << "\n";
		},
		[&](categories::beast)           { logger << "beast\n"; }
	}, category);

	logger.end_message();
}

// vector index is item ID
std::vector<std::optional<lang::price_data>> parse_compact(std::string_view compact_json, log::logger& logger)
{
	nlohmann::json json = nlohmann::json::parse(compact_json);

	if (!json.is_array())
	{
		throw network::poe_watch::json_parse_error("compact JSON must be an array but it is not");
	}

	std::vector<std::optional<lang::price_data>> item_prices;
	item_prices.resize(32768); // expect about 30 000 items
	int max_id = 0;
	for (const auto& item : json)
	{
		const int id = item.at("id").get<int>();

		if (id > static_cast<int>(item_prices.size())) // C++20: [[unlikely]]
		{
			max_id = id;
			item_prices.resize(max_id * 2);
		}
		else if (id > max_id)
		{
			max_id = id;
		}

		if (item_prices[id].has_value()) // C++20: [[unlikely]]
		{
			logger.warning() << "A price data entry with duplicated ID has been found, ID = " << id;
			continue;
		}

		item_prices[id] = lang::price_data{
			item.at("mean")    .get<double>(),
			item.at("median")  .get<double>(),
			item.at("mode")    .get<double>(),
			item.at("min")     .get<double>(),
			item.at("max")     .get<double>(),
			item.at("exalted") .get<double>(),
			item.at("total")   .get<double>(),
			item.at("daily")   .get<double>(),
			item.at("current") .get<double>(),
			item.at("accepted").get<double>()};
	}

	item_prices.resize(max_id);
	return item_prices;
}

[[nodiscard]]
nlohmann::json::string_t dump_json(const nlohmann::json& json)
{
	return json.dump(
		-1,                                        // do not indent
		' ',                                       // indentation character (ignored in case of no indent)
		true,                                      // escape non-ASCII characters as \xXXXX
		nlohmann::json::error_handler_t::replace); // replace invalid UTF-8 sequences with U+FFFD
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
	assert(obj_json.is_object());

	if (auto it = obj_json.find(std::forward<Arg>(arg)); it != obj_json.end())
	{
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
		throw network::poe_watch::json_parse_error("item category should be a string but it is not");

	const auto& category_str = category.get_ref<const nlohmann::json::string_t&>();

	if (category_str == "accessory")
	{
		return categories::accessory{json_to_enum<categories::accessory_type>(group)};
	}
	if (category_str == "armour")
	{
		return categories::armour{json_to_enum<categories::armour_type>(group)};
	}
	if (category_str == "card")
	{
		return categories::divination_card{};
	}
	if (category_str == "currency")
	{
		return categories::currency{json_to_enum<categories::currency_type>(group)};
	}
	if (category_str == "enchantment")
	{
		return categories::enchantment{json_to_enum<categories::enchantment_type>(group)};
	}
	if (category_str == "flask")
	{
		return categories::flask{};
	}
	if (category_str == "gem")
	{
		const auto gem_lvl      = entry.at("gemLevel")  .get<int>();
		const auto gem_quality  = entry.at("gemQuality").get<int>();
		const auto is_corrupted = entry.at("gemIsCorrupted").get<bool>();

		return categories::gem{json_to_enum<categories::gem_type>(group), gem_lvl, gem_quality, is_corrupted};
	}
	if (category_str == "jewel")
	{
		return categories::jewel{};
	}
	if (category_str == "map")
	{
		const auto series = get_optional_subelement<int>(entry, "mapSeries");
		const auto tier   = get_optional_subelement<int>(entry, "mapTier");

		return categories::map{json_to_enum<categories::map_type>(group), series, tier};
	}
	if (category_str == "prophecy")
	{
		return categories::prophecy{};
	}
	if (category_str == "weapon")
	{
		return categories::weapon{json_to_enum<categories::weapon_type>(group)};
	}
	if (category_str == "base")
	{
		const auto is_shaper = entry.at("baseIsShaper").get<bool>();
		const auto is_elder  = entry.at("baseIsElder") .get<bool>();

		if (is_shaper && is_elder)
		{
			throw network::poe_watch::json_parse_error("item can not be shaper and elder at the same time");
		}

		const auto influence =
			is_shaper ? categories::base::influence_type::shaper :
			is_elder  ? categories::base::influence_type::elder  :
			            categories::base::influence_type::none;

		const auto ilvl = entry.at("baseItemLevel").get<int>();
		return categories::base{json_to_enum<categories::base_type>(group), influence, ilvl};
	}
	if (category_str == "beast")
	{
		return categories::beast{};
	}

	throw network::poe_watch::json_parse_error("item has unrecognized category: " + category_str);
}

[[nodiscard]]
item parse_item(const nlohmann::json& item_json)
{
	assert(item_json.is_object());

	return item{
		item_json.at("id").get<int>(),
		item_json.at("name").get<std::string>(),
		get_optional_subelement<std::string>(item_json, "type"),
		parse_item_frame(item_json.at("frame")),
		get_optional_subelement<int>(item_json, "stackSize"),
		get_optional_subelement<int>(item_json, "linkCount"),
		item_json.find("variation") != item_json.end(),
		parse_item_category(item_json)};
}

std::vector<item> parse_itemdata(std::string_view itemdata_json, log::logger& logger)
{
	nlohmann::json json = nlohmann::json::parse(itemdata_json);

	if (!json.is_array())
	{
		throw network::poe_watch::json_parse_error("itemdata JSON must be an array but it is not");
	}

	std::vector<item> items;
	items.reserve(32768); // expect about 30 000 items
	for (const auto& entry : json)
	{
		try
		{
			items.push_back(parse_item(entry));
		}
		catch (const network::poe_watch::json_parse_error& e)
		{
			logger.warning() << "failed to parse item entry in itemdata JSON: " << e.what() << ", skipping this item";
			logger.info() << "entry: " << dump_json(entry);
		}
		catch (const nlohmann::json::exception& e)
		{
			logger.warning() << "failed to parse item entry in itemdata JSON: " << e.what() << ", skipping this item";
			logger.info() << "entry: " << dump_json(entry);
		}
	}

	return items;
}

} // namespace

namespace fs::network::poe_watch
{

std::vector<lang::league> parse_league_info(std::string_view league_json)
{
	nlohmann::json json = nlohmann::json::parse(league_json);

	if (!json.is_array())
	{
		throw network::poe_watch::json_parse_error("league JSON must be an array but it is not");
	}

	std::vector<lang::league> leagues;
	for (const auto& item : json)
	{
		leagues.push_back(
			lang::league{
				item.at("id").get<int>(),
				item.at("name").get<std::string>(),
				item.at("display").get<std::string>()});
	}

	std::sort(leagues.begin(), leagues.end(), [](const lang::league& left, const lang::league& right)
	{
		return left.id < right.id;
	});

	return leagues;
}

lang::item_price_data parse_item_prices(std::string_view itemdata_json, std::string_view compact_json, log::logger& logger)
{
	logger.info() << "parsing item prices";
	std::vector<std::optional<lang::price_data>> item_prices = parse_compact(compact_json, logger);
	if (item_prices.empty())
		throw network::poe_watch::json_parse_error("parsed empty list of item prices");

	logger.info() << "parsing item data";
	std::vector<item> itemdata = parse_itemdata(itemdata_json, logger);
	if (itemdata.empty())
		throw network::poe_watch::json_parse_error("parsed empty list of item data");

	logger.info() << "item entries: " << static_cast<int>(itemdata.size());

	lang::item_price_data result;
	for (item& i : itemdata)
	{
		if (i.id > static_cast<int>(item_prices.size()) || !item_prices[i.id].has_value())
		{
			++result.count_of_items_without_price_data;
			continue;
		}

		const lang::price_data& price_data = *item_prices[i.id];
		using lang::elementary_item;
		using lang::unique_item;
		using lang::base_type_item;

		if (std::holds_alternative<categories::currency>(i.category))
		{
			const auto& curr = std::get<categories::currency>(i.category);
			if (curr.type == +categories::currency_type::essence)
			{
				result.essences.push_back(elementary_item{std::move(i.name), price_data});
			}
			else if (curr.type == +categories::currency_type::piece)
			{
				result.pieces.push_back(elementary_item{std::move(i.name), price_data});
			}
			else if (curr.type == +categories::currency_type::net)
			{
				result.nets.push_back(elementary_item{std::move(i.name), price_data});
			}
			else if (curr.type == +categories::currency_type::vial)
			{
				result.vials.push_back(elementary_item{std::move(i.name), price_data});
			}
			else if (curr.type == +categories::currency_type::fossil)
			{
				result.fossils.push_back(elementary_item{std::move(i.name), price_data});
			}
			else if (curr.type == +categories::currency_type::resonator)
			{
				result.resonators.push_back(elementary_item{std::move(i.name), price_data});
			}
		}
		else if (std::holds_alternative<categories::divination_card>(i.category))
		{
			result.divination_cards.push_back(elementary_item{std::move(i.name), price_data});
		}
		else if (std::holds_alternative<categories::prophecy>(i.category))
		{
			result.prophecies.push_back(elementary_item{std::move(i.name), price_data});
		}
		else if (std::holds_alternative<categories::map>(i.category))
		{
			const auto& map = std::get<categories::map>(i.category);
			if (map.type == +categories::map_type::scarab)
			{
				result.scarabs.push_back(elementary_item{std::move(i.name), price_data});
			}
		}
		else if (std::holds_alternative<categories::base>(i.category))
		{
			const auto& base = std::get<categories::base>(i.category);

			if (base.influence == categories::base::influence_type::none)
			{
				result.bases_without_influence.push_back(base_type_item{base.ilvl, std::move(i.name), price_data});
			}
			else if (base.influence == categories::base::influence_type::shaper)
			{
				result.bases_shaper.push_back(base_type_item{base.ilvl, std::move(i.name), price_data});
			}
			else if (base.influence == categories::base::influence_type::elder)
			{
				result.bases_elder.push_back(base_type_item{base.ilvl, std::move(i.name), price_data});
			}
			else
			{
				logger.warning() << "A base type item with unknown influence type has been found";
				i.log_info(logger);
				continue;
			}
		}
		else if (std::holds_alternative<categories::beast>(i.category))
		{
			// ignore beasts for now
			continue;
		}
		else if (i.frame == +frame_type::unique || i.frame == +frame_type::relic)
		{
			if (!i.base_type)
			{
				logger.warning() << "A unique item without base type has been found";
				i.log_info(logger);
				continue;
			}

			if (*i.base_type == "")
			{
				logger.warning() << "A unique item with empty string base type has been found";
				i.log_info(logger);
				continue;
			}

			if (i.links)
			{
				// skip uniques which have links - we care about sole unique item value, not linked items
				continue;
			}

			if (std::holds_alternative<categories::armour>(i.category))
			{
				result.ambiguous_unique_armours.push_back(unique_item{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::weapon>(i.category))
			{
				result.ambiguous_unique_weapons.push_back(unique_item{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::accessory>(i.category))
			{
				result.ambiguous_unique_accessories.push_back(unique_item{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::jewel>(i.category))
			{
				result.ambiguous_unique_jewels.push_back(unique_item{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::flask>(i.category))
			{
				result.ambiguous_unique_flasks.push_back(unique_item{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::map>(i.category))
			{
				result.ambiguous_unique_maps.push_back(unique_item{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}

			logger.warning() << "A unique item has been skipped because it's category was not recognized";
			i.log_info(logger);
		}
	}

	const auto compare_by_mean_price = [](const auto& left, const auto& right)
	{
		return left.price_data.mean < right.price_data.mean;
	};

	const auto compare_by_ilvl = [](const auto& left, const auto& right)
	{
		return left.ilvl < right.ilvl;
	};

	std::sort(result.divination_cards.begin(),        result.divination_cards.end(),        compare_by_mean_price);
	std::sort(result.prophecies.begin(),              result.prophecies.end(),              compare_by_mean_price);
	std::sort(result.essences.begin(),                result.essences.end(),                compare_by_mean_price);
	std::sort(result.leaguestones.begin(),            result.leaguestones.end(),            compare_by_mean_price);
	std::sort(result.pieces.begin(),                  result.pieces.end(),                  compare_by_mean_price);
	std::sort(result.nets.begin(),                    result.nets.end(),                    compare_by_mean_price);
	std::sort(result.vials.begin(),                   result.vials.end(),                   compare_by_mean_price);
	std::sort(result.fossils.begin(),                 result.fossils.end(),                 compare_by_mean_price);
	std::sort(result.scarabs.begin(),                 result.scarabs.end(),                 compare_by_mean_price);

	std::sort(result.bases_without_influence.begin(), result.bases_without_influence.end(), compare_by_ilvl);
	std::sort(result.bases_shaper.begin(),            result.bases_shaper.end(),            compare_by_ilvl);
	std::sort(result.bases_elder.begin(),             result.bases_elder.end(),             compare_by_ilvl);

	/*
	 * Some unique (and relic) items share the same base type, eg "Headhunter Leather Belt" and "Wurm's Molt Leather Belt".
	 * Unfortunately item filters can only filter by base type.
	 *
	 * I tried the way with assuming that each ambiguous base is an expensive item but it turns out that
	 * a quite large amount of uniques share the same base type which results in a lot of false alarms
	 * (Inpulsa shares base with Tinkerskin, Starforge with Oro's).
	 *
	 * Additionally, items from some categories are almost always ambiguous (jewels, rings, amulets).
	 *
	 * We separate ambiguous and unambiguous base types to allow separate filter rules for 'definitely an expensive item' and
	 * 'maybe an expensive item'.
	 */
	const auto extract_unambiguous_items = [](std::vector<lang::unique_item>& ambiguous_items)
		-> std::vector<lang::unique_item>
	{
		std::sort(ambiguous_items.begin(), ambiguous_items.end(), [](const auto& left, const auto& right)
		{
			return left.base_type_name < right.base_type_name;
		});

		// now items with the same base type are next to each other, just move out unique (not repeated) bases
		std::vector<lang::unique_item> unambiguous_items;
		const auto last = utility::remove_unique(
			ambiguous_items.begin(),
			ambiguous_items.end(),
			std::back_inserter(unambiguous_items),
			[](const auto& left, const auto& right)
			{
				return left.base_type_name == right.base_type_name;
			}).first;
		ambiguous_items.erase(last, ambiguous_items.end()); // note: this is the erase-remove idiom

		return unambiguous_items;
	};

	result.unambiguous_unique_armours     = extract_unambiguous_items(result.ambiguous_unique_armours);
	result.unambiguous_unique_weapons     = extract_unambiguous_items(result.ambiguous_unique_weapons);
	result.unambiguous_unique_accessories = extract_unambiguous_items(result.ambiguous_unique_accessories);
	result.unambiguous_unique_jewels      = extract_unambiguous_items(result.ambiguous_unique_jewels);
	result.unambiguous_unique_flasks      = extract_unambiguous_items(result.ambiguous_unique_flasks);
	result.unambiguous_unique_maps        = extract_unambiguous_items(result.ambiguous_unique_maps);
	std::sort(result.unambiguous_unique_armours.begin(),     result.unambiguous_unique_armours.end(),     compare_by_mean_price);
	std::sort(result.unambiguous_unique_weapons.begin(),     result.unambiguous_unique_weapons.end(),     compare_by_mean_price);
	std::sort(result.unambiguous_unique_accessories.begin(), result.unambiguous_unique_accessories.end(), compare_by_mean_price);
	std::sort(result.unambiguous_unique_jewels.begin(),      result.unambiguous_unique_jewels.end(),      compare_by_mean_price);
	std::sort(result.unambiguous_unique_flasks.begin(),      result.unambiguous_unique_flasks.end(),      compare_by_mean_price);
	std::sort(result.unambiguous_unique_maps.begin(),        result.unambiguous_unique_maps.end(),        compare_by_mean_price);

	logger.info() << "items without associated price: " << result.count_of_items_without_price_data;

	return result;
}

}
