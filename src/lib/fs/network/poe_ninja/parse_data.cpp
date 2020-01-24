#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/exceptions.hpp>
#include <fs/utility/dump_json.hpp>
#include <fs/lang/item_price_data.hpp>
#include <fs/log/logger.hpp>

#include <nlohmann/json.hpp>

#include <vector>
#include <utility>

namespace
{

using namespace fs;

bool is_low_confidence(int count)
{
	return count < 5;
}

template <typename F>
void for_each_item(std::string_view json_str, log::logger& logger, F f)
{
	nlohmann::json json = nlohmann::json::parse(json_str);
	const auto& lines = json.at("lines");
	for (const auto& item : lines) {
		try {
			f(item);
		}
		catch (const network::json_parse_error& e) {
			logger.warning() << "failed to parse item entry: " << e.what()
				<< ", skipping this item: " << utility::dump_json(item);
		}
		catch (const nlohmann::json::exception& e) {
			logger.warning() << e.what() << ", caused by this item: " << utility::dump_json(item);
		}
	}
}

[[nodiscard]]
std::string get_item_name(const nlohmann::json& item)
{
	return item.at("name").get<std::string>();
}

[[nodiscard]]
lang::price_data get_item_price_data(const nlohmann::json& item)
{
	return lang::price_data{
		item.at("chaosValue").get<double>(),
		is_low_confidence(item.at("count").get<int>())
	};
}

[[nodiscard]]
lang::elementary_item get_elementary_item_data(const nlohmann::json& item)
{
	return lang::elementary_item{
		get_item_price_data(item),
		get_item_name(item)
	};
}

[[nodiscard]] std::vector<lang::elementary_item>
parse_elementary_items(std::string_view json_str, log::logger& logger)
{
	std::vector<lang::elementary_item> result;

	for_each_item(json_str, logger, [&](const auto& item) {
		result.emplace_back(get_elementary_item_data(item));
	});

	return result;
}

[[nodiscard]] std::vector<lang::divination_card>
parse_divination_cards(std::string_view json_str, log::logger& logger)
{
	std::vector<lang::divination_card> result;

	for_each_item(json_str, logger, [&](const nlohmann::json& item) {
		result.emplace_back(
			get_elementary_item_data(item),
			item.at("stackSize").get<int>()
		);
	});

	return result;
}

[[nodiscard]] std::vector<lang::gem>
parse_gems(std::string_view json_str, log::logger& logger)
{
	std::vector<lang::gem> result;

	for_each_item(json_str, logger, [&](const nlohmann::json& item) {
		result.emplace_back(
			get_elementary_item_data(item),
			item.at("gemLevel").get<int>(),
			item.at("gemQuality").get<int>(),
			item.at("corrupted").get<bool>()
		);
	});

	return result;
}

[[nodiscard]] std::vector<lang::base>
parse_bases(std::string_view json_str, log::logger& logger)
{
	std::vector<lang::base> result;

	for_each_item(json_str, logger, [&](const nlohmann::json& item) {
		const auto& item_influence = item.at("variant");
		// yes, not really a proper name but poe.ninja reuses some fields for other purposes
		const auto item_level = item.at("levelRequired").get<int>();

		if (item_influence.is_null()) {
			result.emplace_back(
				get_elementary_item_data(item),
				item_level,
				false, false, false, false, false, false
			);
		}
		else {
			const auto& infl = item_influence.get_ref<const nlohmann::json::string_t&>();

			// as of now, poe.ninja only reports singly-influenced items
			// we can just compare the influence string
			result.emplace_back(
				get_elementary_item_data(item),
				item_level,
				infl == "Shaper",
				infl == "Elder",
				infl == "Crusader",
				infl == "Redeemer",
				infl == "Hunter",
				infl == "Warlord"
			);
		}
	});

	return result;
}

void parse_and_fill_uniques(
	std::string_view uniques_json,
	lang::unique_item_price_data& uniques,
	log::logger& logger)
{
	for_each_item(uniques_json, logger, [&](const nlohmann::json& item) {
		// skip uniques which are linked
		if (item.at("links").get<int>() == 6) {
			return;
		}

		// skip uniques which are relics
		// currently the only way to determine a unique item is relic is checking
		// the pattern inside "detailsId" field of the item
		if (const auto& details = item.at("detailsId").get_ref<const nlohmann::json::string_t&>();
			details.find("-relic") != std::string::npos)
		{
			return;
		}

		// skip uniques which do not drop (eg fated items) - this will reduce ambiguity and
		// not pollute the filter with items we would not care for
		const auto& name = item.at("name").get_ref<const nlohmann::json::string_t&>();
		if (lang::is_undroppable_unique(name)) {
			return;
		}

		const auto& base_type = item.at("baseType").get_ref<const nlohmann::json::string_t&>();
		uniques.add_item(base_type, lang::elementary_item{get_item_price_data(item), name});
	});
}

} // namespace

namespace fs::network::poe_ninja
{

lang::item_price_data parse_item_price_data(const api_item_price_data& jsons, log::logger& logger)
{
	lang::item_price_data result;

	result.divination_cards = parse_divination_cards(jsons.divination_card, logger);

	result.oils = parse_elementary_items(jsons.oil, logger);
	result.incubators = parse_elementary_items(jsons.incubator, logger);
	result.essences = parse_elementary_items(jsons.essence, logger);
	result.fossils = parse_elementary_items(jsons.fossil, logger);
	result.prophecies = parse_elementary_items(jsons.prophecy, logger);
	result.resonators = parse_elementary_items(jsons.resonator, logger);
	result.scarabs = parse_elementary_items(jsons.scarab, logger);
	result.helmet_enchants = parse_elementary_items(jsons.helmet_enchant, logger);

	result.gems = parse_gems(jsons.skill_gem, logger);

	result.bases = parse_bases(jsons.base_type, logger);

	parse_and_fill_uniques(jsons.unique_armour, result.unique_eq, logger);
	parse_and_fill_uniques(jsons.unique_weapon, result.unique_eq, logger);
	parse_and_fill_uniques(jsons.unique_accessory, result.unique_eq, logger);

	parse_and_fill_uniques(jsons.unique_flask, result.unique_flasks, logger);

	parse_and_fill_uniques(jsons.unique_jewel, result.unique_jewels, logger);

	parse_and_fill_uniques(jsons.unique_map, result.unique_maps, logger);

	/*
	 * not all jsons are being read but:
	 * - we do not care about non-unique maps - people filter them by tier
	 * - we do not care about beasts - they do not drop
	 */
	return result;
}

}
