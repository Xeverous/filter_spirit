#include "itemdata/parse_json.hpp"
#include "itemdata/exceptions.hpp"
#include "utility/better_enum.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <optional>
#include <variant>
#include <type_traits>
#include <utility>

#include <iostream> // TODO refactor logging and get rid of this

namespace
{

enum class frame_type
{
	// NOTE: values are exactly in this order - they must match
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

	unknown // must be last
};

namespace categories // avoids name conflicts with types in fs::itemdata
{

	BETTER_ENUM(accessory_type, int, amulet, belt, ring, unknown)
	struct accessory { accessory_type type; };

	BETTER_ENUM(armour_type, int, boots, chest, gloves, helmet, quiver, shield, unknown)
	struct armour { armour_type type; };

	struct divination_card {};

	BETTER_ENUM(currency_type, int, currency, essense, piece, fossil, resonator, vial, net, unknown)
	struct currency { currency_type type; };

	struct enchantment {}; // we do not care about lab enchants, Path of Exile filters don't support filtering them
	struct flask {};

	BETTER_ENUM(gem_type, int, skill, support, vaal, unknown)
	struct gem { gem_type type; };

	struct jewel {};

	BETTER_ENUM(map_type, int, map, fragment, unique, scarab, leaguestone, unknown)
	struct map { map_type type; };

	struct prophecy {};

	BETTER_ENUM(weapon_type, int, bow, claw, dagger, oneaxe, onemace, onesword, rod, sceptre, staff, twoaxe, twomace, twosword, wand, unknown)
	struct weapon { weapon_type type; };

	BETTER_ENUM(base_type, int, ring, belt, amulet, helmet, chest, gloves, boots, onemace, sceptre, bow, wand, onesword, claw, shield, dagger, twosword, staff, oneaxe, quiver, twoaxe, twomace, jewel, unknown)
	struct base { base_type type; };

	struct unknown {};

} // namespace categories

using item_category_variant = std::variant<
	categories::accessory, categories::armour, categories::divination_card, categories::currency, categories::enchantment, categories::flask, categories::gem, categories::jewel, categories::map, categories::prophecy, categories::weapon, categories::base, categories::unknown>;

struct unique_with_variations {};
struct shaper_item {};
struct elder_item {};

using item_variation_variant = std::variant<
	unique_with_variations, shaper_item, elder_item>;

struct item
{
	int id = 0;
	std::string name;                     // item main name, eg "Tabula Rasa"
	std::optional<std::string> base_type; // item base type, eg "Simple Robe", null for non-wearable items
	frame_type frame;                  // frame displayed in game UI
	std::optional<int> map_tier;       // only for maps
	std::optional<int> max_stack_size; // only for stackable items
	std::optional<int> gem_lvl;        // only for gems
	std::optional<int> gem_quality;    // only for gems
	std::optional<bool> gem_corrupted; // only for gems
	std::optional<int> links;
	std::optional<int> ilvl;
	std::optional<item_variation_variant> variation; // unique item variation (eg Vessel of Vinktar has 4 variations) OR shaper/elder
	// url icon_path; // we do not care about item icons
	item_category_variant category; // combined category and group
};

// vector index is item ID
std::vector<std::optional<fs::itemdata::price_data>> parse_compact(std::string_view compact_json)
{
	nlohmann::json json = nlohmann::json::parse(compact_json);

	if (!json.is_array())
	{
		throw std::runtime_error("compact JSON must be an array but it is not");
	}

	std::vector<std::optional<fs::itemdata::price_data>> item_prices;
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
			// TODO warn about duplicated ID
			// log the problem, but still continue
			continue;
		}

		item_prices[id] = fs::itemdata::price_data{
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

template <typename T>
std::optional<T> parse_single_json_element(const nlohmann::json& json)
{
	static_assert(sizeof(T) == 0, "missing overload for this T");
}

template <>
std::optional<std::string> parse_single_json_element(const nlohmann::json& json)
{
	if (json.is_string())
		return json.get<std::string>();
	else
		return std::nullopt;
}

template <>
std::optional<int> parse_single_json_element(const nlohmann::json& json)
{
	if (json.is_number())
		return json.get<int>();
	else
		return std::nullopt;
}

template <>
std::optional<bool> parse_single_json_element(const nlohmann::json& json)
{
	if (json.is_boolean())
		return json.get<bool>();
	else
		return std::nullopt;
}

std::optional<item_variation_variant> parse_item_variation(const nlohmann::json& json)
{
	if (json.is_null())
		return std::nullopt;

	if (!json.is_string())
		throw fs::itemdata::unknown_item_variation(
			json.dump( // dump the sub-json where error happened
				-1,                                         // do not indent
				' ',                                        // indentation character (ignored in case of no indent)
				true,                                       // escape non-ASCII characters as \xXXXX
				nlohmann::json::error_handler_t::replace)); // replace invalid UTF-8 sequences with U+FFFD

	if (json == "shaper")
		return shaper_item{};

	if (json == "elder")
		return elder_item{};

	return unique_with_variations{};
}

frame_type parse_item_frame(const nlohmann::json& json)
{
	if (!json.is_number())
		return frame_type::unknown;

	const int val = json.get<int>();

	if (val < 0 || val >= static_cast<int>(frame_type::unknown))
		return frame_type::unknown;

	return static_cast<frame_type>(val);
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

item_category_variant parse_item_category_and_group(const nlohmann::json& category, const nlohmann::json& group)
{
	if (!category.is_string())
		return categories::unknown{};

	const auto& category_str = category.get_ref<const nlohmann::json::string_t&>();

	if (category_str == "accessory")
		return categories::accessory{json_to_enum<categories::accessory_type>(group)};
	if (category_str == "armour")
		return categories::armour{json_to_enum<categories::armour_type>(group)};
	if (category_str == "card")
		return categories::divination_card{};
	if (category_str == "currency")
		return categories::currency{json_to_enum<categories::currency_type>(group)};
	if (category_str == "enchantment")
		return categories::enchantment{};
	if (category_str == "flask")
		return categories::flask{};
	if (category_str == "gem")
		return categories::gem{json_to_enum<categories::gem_type>(group)};
	if (category_str == "jewel")
		return categories::jewel{};
	if (category_str == "map")
		return categories::map{json_to_enum<categories::map_type>(group)};
	if (category_str == "prophecy")
		return categories::prophecy{};
	if (category_str == "weapon")
		return categories::weapon{json_to_enum<categories::weapon_type>(group)};
	if (category_str == "base")
		return categories::base{json_to_enum<categories::base_type>(group)};

	// TODO log unknown category
	return categories::unknown{};
}

std::vector<item> parse_itemdata(std::string_view itemdata_json)
{
	nlohmann::json json = nlohmann::json::parse(itemdata_json);

	if (!json.is_array())
	{
		// TODO log invalid json
	}

	std::vector<item> items;
	items.reserve(32768); // expect about 30 000 items
	for (const auto& entry : json)
	{
		items.push_back(item{
			entry.at("id").get<int>(),
			entry.at("name").get<std::string>(),
			parse_single_json_element<std::string>(entry.at("type")),
			parse_item_frame(entry.at("frame")),
			parse_single_json_element<int>(entry.at("tier")),
			parse_single_json_element<int>(entry.at("stack")),
			parse_single_json_element<int>(entry.at("lvl")),
			parse_single_json_element<int>(entry.at("quality")),
			parse_single_json_element<bool>(entry.at("corrupted")),
			parse_single_json_element<int>(entry.at("links")),
			parse_single_json_element<int>(entry.at("ilvl")),
			parse_item_variation(entry.at("variation")),
			parse_item_category_and_group(entry.at("category"), entry.at("group"))});
	}

	return items;
}

} // namespace

namespace fs::itemdata
{

std::vector<league> parse_league_info(std::string_view league_json)
{
	nlohmann::json json = nlohmann::json::parse(league_json);

	if (!json.is_array())
	{
		// TODO log invalid json
	}

	std::vector<league> leagues;
	for (const auto& item : json)
	{
		leagues.push_back(
			league{
				item.at("id").get<int>(),
				item.at("name").get<std::string>(),
				item.at("display").get<std::string>()});
	}

	std::sort(leagues.begin(), leagues.end(), [](const league& left, const league& right)
	{
		return left.id < right.id;
	});

	return leagues;
}

item_price_data parse_item_prices(std::string_view itemdata_json, std::string_view compact_json)
{
	std::cout << "parsing item prices" << std::endl;
	std::vector<std::optional<price_data>> item_prices = parse_compact(compact_json);
	if (item_prices.empty())
		throw std::runtime_error("parsed empty list of item prices");

	std::cout << "got " << item_prices.size() << " entries" << std::endl;


	std::cout << "parsing item data" << std::endl;
	std::vector<item> itemdata = parse_itemdata(itemdata_json);
	if (itemdata.empty())
		throw std::runtime_error("parsed empty list of item data");

	std::cout << "got " << itemdata.size() << " entries" << std::endl;

	item_price_data result;
	for (item& i : itemdata)
	{
		if (i.id > static_cast<int>(item_prices.size()) || !item_prices[i.id].has_value())
		{
			// TODO log thet an item without associated price data has been encountered
			continue;
		}

		const fs::itemdata::price_data& price_data = *item_prices[i.id];

		if (std::holds_alternative<categories::divination_card>(i.category))
		{
			result.divination_cards.push_back(divination_card{std::move(i.name), price_data});
		}
		else if (std::holds_alternative<categories::prophecy>(i.category))
		{
			result.prophecies.push_back(prophecy{std::move(i.name), price_data});
		}
		else if (std::holds_alternative<categories::base>(i.category))
		{
			if (!i.ilvl.has_value())
			{
				// TODO log that a base without item level has been encountered
				continue;
			}

			if (!i.variation.has_value()) // non-influenced item
			{
				result.bases_without_influence.push_back(base_without_influence{*i.ilvl, std::move(i.name), price_data});
			}
			else if (std::holds_alternative<shaper_item>(*i.variation))
			{
				result.bases_shaper.push_back(base_shaper{*i.ilvl, std::move(i.name), price_data});
			}
			else if (std::holds_alternative<elder_item>(*i.variation))
			{
				result.bases_elder.push_back(base_elder{*i.ilvl, std::move(i.name), price_data});
			}
			else
			{
				// TODO log that a base type entry with unknown variation has been encountered
				continue;
			}
		}
		else if (i.frame == frame_type::unique)
		{
			if (!i.base_type.has_value())
			{
				// TODO log that a unique item without base type has been encountered
				continue;
			}

			if (i.links.has_value())
			{
				// skip uniques which have links - we care about unique item value, 6L items are filtered earlier
				continue;
			}

			if (std::holds_alternative<categories::armour>(i.category))
			{
				result.unique_armours.push_back(unique_armour{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::weapon>(i.category))
			{
				result.unique_weapons.push_back(unique_weapon{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::accessory>(i.category))
			{
				result.unique_accessories.push_back(unique_accessory{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::jewel>(i.category))
			{
				result.unique_jewels.push_back(unique_jewel{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::flask>(i.category))
			{
				result.unique_flasks.push_back(unique_flask{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}
			if (std::holds_alternative<categories::map>(i.category))
			{
				result.unique_maps.push_back(unique_map{std::move(i.name), std::move(*i.base_type), price_data});
				continue;
			}

			// TODO log that a unique was skipped
		}
		else if (i.frame == frame_type::relic)
		{
			if (!i.base_type.has_value())
			{
				// TODO log that a relic item without base type has been encountered
				continue;
			}

			if (i.links.has_value())
			{
				// skip relics which have links - we care about relic item value, 6L items are filtered earlier
				continue;
			}

			result.relic_items.push_back(relic_item{std::move(i.name), std::move(*i.base_type), price_data});
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
	std::sort(result.bases_without_influence.begin(), result.bases_without_influence.end(), compare_by_ilvl);
	std::sort(result.bases_shaper.begin(),            result.bases_shaper.end(),            compare_by_ilvl);
	std::sort(result.bases_elder.begin(),             result.bases_elder.end(),             compare_by_ilvl);

	/*
	 * Some unique (and relic) items share the same base type, eg "Headhunter Leather Belt" and "Wurm's Molt Leather Belt".
	 * Unfortunately item filters can only filter by base type, so we have 2 choices:
	 * - assume the found item has the value of less expensive unique
	 * - assume the found item has the value of more expensive unique
	 * Being cautious, we assume that the item is expensive - better to have some false alarms than a missed Headhunter
	 *
	 * The lambda will remove cheaper items that share the same base type with more expensive ones
	 */
	const auto remove_duplicated_bases_with_lower_price = [&](auto& items)
	{
		std::sort(items.begin(), items.end(), [](const auto& left, const auto& right)
		{
			return left.price_data.mean > right.price_data.mean;
		});
		std::stable_sort(items.begin(), items.end(), [](const auto& left, const auto& right)
		{
			return left.base_type_name < right.base_type_name;
		});

		// now items with the same base type are next to each other
		// each group of same-base-type items is sorted descending by price

		// std::unique is specified to remove all duplicates except the first
		// thus after the call to erase, only 1 (the most expensive) item of each base type is present
		// note: this is the erase-remove idiom
		const auto last = std::unique(items.begin(), items.end(), [](const auto& left, const auto& right)
		{
			return left.base_type_name == right.base_type_name;
		});
		items.erase(last, items.end());
	};

	remove_duplicated_bases_with_lower_price(result.unique_armours);
	remove_duplicated_bases_with_lower_price(result.unique_weapons);
	remove_duplicated_bases_with_lower_price(result.unique_accessories);
	remove_duplicated_bases_with_lower_price(result.unique_jewels);
	remove_duplicated_bases_with_lower_price(result.unique_flasks);
	remove_duplicated_bases_with_lower_price(result.unique_maps);
	std::sort(result.unique_armours.begin(), result.unique_armours.end(), compare_by_mean_price);
	std::sort(result.unique_weapons.begin(), result.unique_weapons.end(), compare_by_mean_price);
	std::sort(result.unique_accessories.begin(), result.unique_accessories.end(), compare_by_mean_price);
	std::sort(result.unique_jewels.begin(), result.unique_jewels.end(), compare_by_mean_price);
	std::sort(result.unique_flasks.begin(), result.unique_flasks.end(), compare_by_mean_price);
	std::sort(result.unique_maps.begin(), result.unique_maps.end(), compare_by_mean_price);

	remove_duplicated_bases_with_lower_price(result.relic_items);
	std::sort(result.relic_items.begin(),  result.relic_items.end(),  compare_by_mean_price);

	return result;
}

}
