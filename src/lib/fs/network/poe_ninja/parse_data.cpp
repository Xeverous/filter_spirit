#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/exceptions.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/math.hpp>
#include <fs/utility/dump_json.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/log/logger.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <string_view>
#include <vector>
#include <tuple>

namespace fs::network::poe_ninja {

/**
 * poe.ninja uses 2 data models to report items.
 *
 * - exchange: {"core": {...}, "lines": [...], "items": [...]}
 * - stash: {"lines"; [...]}
 *
 * The "core" part is identical in every exchange-model JSON downloaded at the same time.
 * As of writing this, PoE 2 currently only reports currencies and thus only uses the exchange model.
 */

using fs::utility::is_zero;

namespace {

void print_file_parse_error(std::string_view json_str, const char* file_stem, log::logger& logger)
{
	logger.error() << "Could not parse \"" << file_stem << ".json\": (first 200 characters): " << json_str.substr(0u, 200u) << "\n";
}

template <typename F>
void for_each_item_in_items(const nlohmann::json& items, const char* file_stem, log::logger& logger, F f)
{
	for (const auto& item : items) {
		try {
			f(item);
		}
		catch (const network::json_parse_error& e) {
			logger.warning() << "\"" << file_stem << ".json\": failed to parse item entry: " << e.what()
				<< ", skipping this item: " << utility::dump_json(item) << '\n';
		}
		catch (const nlohmann::json::exception& e) {
			logger.warning() << "\"" << file_stem << ".json\":" << e.what()
				<< ", ignoring this item: " << utility::dump_json(item) << '\n';
		}
	}
}

enum class primary_currency { chaos, exalted, divine };
struct currency_info
{
	primary_currency primary;
	lang::market::currency_exchange_rates rates;
};

std::optional<primary_currency> parse_primary_currency(const nlohmann::json& json, const char* file_stem, log::logger& logger)
{
	const auto it_core = json.find("core");
	if (it_core == json.end()) {
		logger.error() << "Could not find \"core\" subobject in \"" << file_stem << ".json\"\n";
		return std::nullopt;
	}

	const auto it_primary = it_core->find("primary");
	if (it_primary == it_core->end()) {
		logger.error() << "Could not find \"core\".\"primary\" subobject in \"" << file_stem << ".json\"\n";
		return std::nullopt;
	}

	const auto& primary_currency = it_primary->get_ref<const std::string&>();

	if (primary_currency == "chaos")
		return primary_currency::chaos;
	else if (primary_currency == "exalted")
		return primary_currency::exalted;
	else if (primary_currency == "divine")
		return primary_currency::divine;
	else {
		logger.error() << "Unknown primary currency: \"" << primary_currency << "\" in \"" << file_stem << ".json\"\n";
		return std::nullopt;
	}
}

std::optional<currency_info> parse_currency_info(const nlohmann::json& json, log::logger& logger)
{
	currency_info info;

	if (auto primary = parse_primary_currency(json, poe1::file_stem_currency, logger); !primary) // poe2 has same filename
		return std::nullopt;
	else
		info.primary = *primary;

	const auto it_core = json.find("core");
	if (it_core == json.end()) {
		logger.error() << "Could not find \"core\" subobject in \"" << poe1::file_stem_currency << ".json\"\n";
		return std::nullopt;
	}

	const auto it_rates = it_core->find("rates");
	if (it_rates == it_core->end()) {
		logger.error() << "Could not find \"core\".\"rates\" subobject in \"" << poe1::file_stem_currency << ".json\"\n";
		return std::nullopt;
	}

	const auto get_rate = [&](const char* name) -> std::optional<double> {
		const auto it = it_rates->find(name);
		if (it == it_rates->end())
			return std::nullopt;
		return it->get<double>();
	};

	if (info.primary == primary_currency::chaos) {
		auto chaos_to_exalted = get_rate("exalted");
		if (chaos_to_exalted)
			info.rates.exalted_to_chaos = 1.0 / *chaos_to_exalted;

		auto chaos_to_divine = get_rate("divine");
		if (chaos_to_divine)
			info.rates.divine_to_chaos = 1.0 / *chaos_to_divine;

		if (chaos_to_exalted && chaos_to_divine)
			info.rates.divine_to_exalted = info.rates.divine_to_chaos / info.rates.exalted_to_chaos;
	}
	else if (info.primary == primary_currency::exalted) {
		auto exalted_to_chaos = get_rate("chaos");
		if (exalted_to_chaos)
			info.rates.exalted_to_chaos = *exalted_to_chaos;

		auto exalted_to_divine = get_rate("divine");
		if (exalted_to_divine)
			info.rates.divine_to_exalted = 1.0 / *exalted_to_divine;

		if (exalted_to_chaos && exalted_to_divine)
			info.rates.divine_to_chaos = info.rates.divine_to_exalted * info.rates.exalted_to_chaos;
	}
	else {
		FS_ASSERT(info.primary == primary_currency::divine);

		auto divine_to_chaos = get_rate("chaos");
		if (divine_to_chaos)
			info.rates.divine_to_chaos = *divine_to_chaos;

		auto divine_to_exalted = get_rate("exalted");
		if (divine_to_exalted)
			info.rates.divine_to_exalted = *divine_to_exalted;

		if (divine_to_chaos && divine_to_exalted)
			info.rates.exalted_to_chaos = info.rates.divine_to_chaos / info.rates.divine_to_exalted;
	}

	/**
	 * With 2 currencies reported in core, 1 exchange rate should be non-zero
	 * With 3 currencies reported in core, 3 exchange rates should be non-zero
	 */
	if (is_zero(info.rates.divine_to_chaos) && is_zero(info.rates.divine_to_exalted) && is_zero(info.rates.exalted_to_chaos)) {
		logger.error() << "Could not find any \"core\".\"rates\".\"*\" subobject in \"" << poe1::file_stem_currency << ".json\"\n";
		return std::nullopt;
	}

	return info;
}

lang::market::price_data compute_item_price(double primary_value, primary_currency primary_type, lang::market::currency_exchange_rates rates)
{
	lang::market::price_data price_data;
	// data from currency exchange API is always high confidence
	// (and this function is only used for such items)
	price_data.confidence = lang::market::confidence_level::high;

	if (primary_type == primary_currency::chaos) {
		price_data.value_chaos = primary_value;

		if (!is_zero(rates.exalted_to_chaos))
			price_data.value_exalted = price_data.value_chaos / rates.exalted_to_chaos;

		if (!is_zero(rates.divine_to_chaos))
			price_data.value_divine  = price_data.value_chaos / rates.divine_to_chaos;
	}
	else if (primary_type == primary_currency::exalted) {
		price_data.value_exalted = primary_value;

		if (!is_zero(rates.exalted_to_chaos))
			price_data.value_chaos  = price_data.value_exalted * rates.exalted_to_chaos;

		if (!is_zero(rates.divine_to_exalted))
			price_data.value_divine = price_data.value_exalted / rates.divine_to_exalted;
	}
	else {
		FS_ASSERT(primary_type == primary_currency::divine);
		price_data.value_divine = primary_value;

		if (!is_zero(rates.divine_to_chaos))
			price_data.value_chaos   = price_data.value_divine * rates.divine_to_chaos;

		if (!is_zero(rates.divine_to_exalted))
			price_data.value_exalted = price_data.value_divine * rates.divine_to_exalted;
	}

	return price_data;
}

[[nodiscard]] std::vector<lang::market::elementary_item>
parse_exchange_items(const nlohmann::json& json, const char* file_stem, lang::market::currency_exchange_rates rates, log::logger& logger)
{
	std::optional<primary_currency> primary = parse_primary_currency(json, file_stem, logger);
	if (!primary)
		return {};

	const auto it_items = json.find("items"); // item id => item description (includes name)
	const auto it_lines = json.find("lines"); // item id => item value

	if (it_lines == json.end() || it_items == json.end()) {
		logger.error() << "Could not parse \"" << file_stem << ".json\": missing \"items\" and/or \"lines\" subobjects\n";
		return {};
	}

	std::unordered_map<std::string, std::string> item_ids_to_names;
	item_ids_to_names.reserve(it_items->size());

	for_each_item_in_items(*it_items, file_stem, logger, [&](const nlohmann::json& item) {
		item_ids_to_names.emplace(item.at("id"), item.at("name"));
	});

	std::vector<lang::market::elementary_item> result;
	for_each_item_in_items(*it_lines, file_stem, logger, [&](const nlohmann::json& item) {
		const auto id = item.at("id").get_ref<const std::string&>();
		const auto name_it = item_ids_to_names.find(id);

		if (name_it == item_ids_to_names.end()) {
			logger.error() << "\"" << file_stem << ".json\": price data for item with id = \"" << id << "\" has no item associated\n";
			return;
		}

		result.push_back(lang::market::elementary_item{
			compute_item_price(item.at("primaryValue").get<double>(), *primary, rates),
			name_it->second
		});
	});

	return result;
}

template <const char* FileStem>
[[nodiscard]] std::vector<lang::market::elementary_item>
parse_exchange_items(const json_file<FileStem, true>& json, lang::market::currency_exchange_rates rates, log::logger& logger)
{
	return parse_exchange_items(nlohmann::json::parse(json.file_content), FileStem, rates, logger);
}

[[nodiscard]] std::pair<lang::market::currency_exchange_rates, std::vector<lang::market::elementary_item>>
parse_exchange_currency(std::string_view json_str, log::logger& logger)
{
	const nlohmann::json json = nlohmann::json::parse(json_str);
	std::optional<currency_info> maybe_info = parse_currency_info(json, logger);
	if (!maybe_info)
		return {};

	lang::market::currency_exchange_rates rates = (*maybe_info).rates;

	// at this point the "rates" object may contain only partial information
	// items may have only a single non-zero price field (the primary currency)
	// in addition to this, the primary currency item can be missing (it would have value == 1.0)
	auto result = parse_exchange_items(json, poe1::file_stem_currency, rates, logger); // poe2 has same filename

	// Fill missing info about c/ex/div rates when:
	// - these items are not reported in "core" (parse_currency_info)
	// - these items are reported in "lines" (parse_exchange_items)

	const auto find_currency_item = [&result](const char* name) -> const lang::market::elementary_item* {
		const auto it = std::find_if(result.begin(), result.end(), [&](const lang::market::elementary_item& item) { return item.name == name; });
		if (it == result.end())
			return nullptr;
		else
			return &*it;
	};

	const auto chaos   = find_currency_item("Chaos Orb");
	const auto exalted = find_currency_item("Exalted Orb");
	const auto divine  = find_currency_item("Divine Orb");

	if (is_zero(rates.divine_to_chaos)) {
		if (chaos && !is_zero(chaos->price.value_divine))
			rates.divine_to_chaos = 1.0 / chaos->price.value_divine;
		else if (exalted && !is_zero(exalted->price.value_chaos) && !is_zero(exalted->price.value_divine))
			rates.divine_to_chaos = exalted->price.value_chaos / exalted->price.value_divine;
		else if (divine && !is_zero(divine->price.value_chaos))
			rates.divine_to_chaos = divine->price.value_chaos;
	}

	if (is_zero(rates.divine_to_exalted)) {
		if (chaos && !is_zero(chaos->price.value_divine) && !is_zero(chaos->price.value_exalted))
			rates.divine_to_exalted = chaos->price.value_exalted / chaos->price.value_divine;
		else if (exalted && !is_zero(exalted->price.value_divine))
			rates.divine_to_exalted = 1.0 / exalted->price.value_divine;
		else if (divine && !is_zero(divine->price.value_exalted))
			rates.divine_to_exalted = divine->price.value_exalted;
	}

	if (is_zero(rates.exalted_to_chaos)) {
		if (chaos && !is_zero(chaos->price.value_exalted))
			rates.exalted_to_chaos = 1.0 / chaos->price.value_exalted;
		else if (exalted && !is_zero(exalted->price.value_chaos))
			rates.exalted_to_chaos = exalted->price.value_chaos;
		else if (divine && !is_zero(divine->price.value_chaos) && !is_zero(divine->price.value_exalted))
			rates.exalted_to_chaos = divine->price.value_chaos / divine->price.value_exalted;
	}

	// update prices of all items, given richer exchange rates information
	for (lang::market::elementary_item& item : result) {
		switch ((*maybe_info).primary) {
			case primary_currency::chaos:
				item.price = compute_item_price(item.price.value_chaos, primary_currency::chaos, rates);
				break;
			case primary_currency::exalted:
				item.price = compute_item_price(item.price.value_exalted, primary_currency::exalted, rates);
				break;
			case primary_currency::divine:
				item.price = compute_item_price(item.price.value_divine, primary_currency::divine, rates);
				break;
		}
	}

	// Sometimes main currencies (Chaos, Exalted, Divine) are not reported. Add them if missing, based on known rates
	if (!chaos) {
		lang::market::price_data price_data;
		price_data.confidence = lang::market::confidence_level::high;
		price_data.value_chaos = 1.0;

		if (!is_zero(rates.exalted_to_chaos))
			price_data.value_exalted = 1.0 / rates.exalted_to_chaos;

		if (!is_zero(rates.divine_to_chaos))
			price_data.value_divine = 1.0 / rates.divine_to_chaos;

		result.push_back(lang::market::elementary_item{price_data, "Chaos Orb"});
	}

	if (!exalted) {
		lang::market::price_data price_data;
		price_data.confidence = lang::market::confidence_level::high;
		price_data.value_exalted = 1.0;

		if (!is_zero(rates.exalted_to_chaos))
			price_data.value_chaos = rates.exalted_to_chaos;

		if (!is_zero(rates.divine_to_exalted))
			price_data.value_divine = 1.0 / rates.divine_to_exalted;

		result.push_back(lang::market::elementary_item{price_data, "Exalted Orb"});
	}

	if (!divine) {
		lang::market::price_data price_data;
		price_data.confidence = lang::market::confidence_level::high;
		price_data.value_divine = 1.0;

		if (!is_zero(rates.divine_to_chaos))
			price_data.value_chaos = rates.divine_to_chaos;

		if (!is_zero(rates.divine_to_exalted))
			price_data.value_exalted = rates.divine_to_exalted;

		result.push_back(lang::market::elementary_item{price_data, "Divine Orb"});
	}

	return std::make_pair(rates, std::move(result));
}

// poe.ninja sorts items by mechanical themes instead of filter item classes.
// For example, many Splinters are grouped with other items from same mechanic or
// put into Fragments even though they are technically "Stackable Currency".
// This function exists to adjust categorization for filter purposes.
void move_item(
	std::string_view name,
	std::vector<lang::market::elementary_item>& from,
	std::vector<lang::market::elementary_item>& to)
{
	const auto it = std::find_if(
		from.begin(),
		from.end(),
		[&](const lang::market::elementary_item& item) { return item.name == name; });

	if (it == from.end())
		return;

	to.push_back(std::move(*it));
	from.erase(it);
}

}

namespace poe1 {

namespace {

lang::market::confidence_level to_confidence_level(int count)
{
	if (count < 5)
		return lang::market::confidence_level::low;
	else if (count < 10)
		return lang::market::confidence_level::medium;
	else
		return lang::market::confidence_level::high;
}

template <typename F>
void for_each_item_in_json(std::string_view json_str, const char* file_stem, log::logger& logger, F f)
{
	const nlohmann::json json = nlohmann::json::parse(json_str);
	const auto it = json.find("lines");

	if (it == json.end()) {
		print_file_parse_error(json_str, file_stem, logger);
		return;
	}

	for_each_item_in_items(*it, file_stem, logger, f);
}

[[nodiscard]] const std::string&
get_stash_item_property_name(const nlohmann::json& item)
{
	return item.at("name").get_ref<const std::string&>();
}

[[nodiscard]] bool
get_stash_item_property_corrupted(const nlohmann::json& item)
{
	if (const auto it = item.find("corrupted"); it == item.end())
		return false;
	else
		return it->get<bool>();
}

[[nodiscard]] int
get_stash_item_property_gem_quality(const nlohmann::json& item)
{
	if (const auto it = item.find("gemQuality"); it == item.end())
		return 0;
	else
		return it->get<int>();
}

[[nodiscard]] int
get_stash_item_property_links(const nlohmann::json& item)
{
	if (const auto it = item.find("links"); it == item.end())
		return 0;
	else
		return it->get<int>();
}

[[nodiscard]] lang::influence_info
get_stash_item_property_influence_info(const nlohmann::json& item)
{
	const auto it = item.find("variant");
	if (it == item.end())
		return {};

	if (it->is_null())
		return {};

	const auto& infl = it->get_ref<const nlohmann::json::string_t&>();

	// poe.ninja reports influence in strings as "X" or "X/Y"
	// so we can safely use string-contains approach
	return lang::influence_info{
		utility::contains(infl, "Shaper"),
		utility::contains(infl, "Elder"),
		utility::contains(infl, "Crusader"),
		utility::contains(infl, "Redeemer"),
		utility::contains(infl, "Hunter"),
		utility::contains(infl, "Warlord")
	};
}

[[nodiscard]] lang::market::price_data
get_stash_item_price_data(const nlohmann::json& item, lang::market::currency_exchange_rates rates)
{
	lang::market::price_data price_data = compute_item_price(item.at("chaosValue").get<double>(), primary_currency::chaos, rates);
	price_data.confidence = to_confidence_level(item.at("count").get<int>());
	return price_data;
}

[[nodiscard]] lang::market::elementary_item
get_stash_item_data(const nlohmann::json& item, lang::market::currency_exchange_rates rates)
{
	return lang::market::elementary_item{
		get_stash_item_price_data(item, rates),
		get_stash_item_property_name(item),
	};
}

[[nodiscard]] std::vector<lang::market::elementary_item>
parse_stash_items(std::string_view json_str, const char* file_stem, lang::market::currency_exchange_rates rates, log::logger& logger)
{
	std::vector<lang::market::elementary_item> result;

	for_each_item_in_json(json_str, file_stem, logger, [&](const auto& item) {
		result.push_back(get_stash_item_data(item, rates));
	});

	return result;
}

template <const char* FileStem>
[[nodiscard]] std::vector<lang::market::elementary_item>
parse_stash_items(const json_file<FileStem>& json, lang::market::currency_exchange_rates rates, log::logger& logger)
{
	return parse_stash_items(json.file_content, FileStem, rates, logger);
}

[[nodiscard]] std::vector<lang::market::poe1::gem>
parse_gems(std::string_view json_str, lang::market::currency_exchange_rates rates, log::logger& logger)
{
	std::vector<lang::market::poe1::gem> result;

	for_each_item_in_json(json_str, file_stem_skill_gem, logger, [&](const nlohmann::json& item) {
		result.emplace_back(
			get_stash_item_data(item, rates),
			item.at("gemLevel").get<int>(),
			get_stash_item_property_gem_quality(item),
			get_stash_item_property_corrupted(item)
		);
	});

	return result;
}

[[nodiscard]] std::vector<lang::market::poe1::base>
parse_bases(std::string_view json_str, lang::market::currency_exchange_rates rates, log::logger& logger)
{
	std::vector<lang::market::poe1::base> result;

	for_each_item_in_json(json_str, file_stem_base_type, logger, [&](const nlohmann::json& item) {
		// yes, not really a proper name but poe.ninja reuses some fields for other purposes
		const auto item_level = item.at("levelRequired").get<int>();
		result.emplace_back(
			get_stash_item_data(item, rates),
			item_level,
			get_stash_item_property_influence_info(item)
		);
	});

	return result;
}

void parse_and_fill_uniques(
	std::string_view uniques_json,
	const char* file_stem,
	lang::market::currency_exchange_rates rates,
	lang::market::poe1::unique_item_price_data& uniques,
	log::logger& logger)
{
	for_each_item_in_json(uniques_json, file_stem, logger, [&](const nlohmann::json& item) {
		// skip uniques which are linked
		if (get_stash_item_property_links(item) == 6) {
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
		const auto& name = get_stash_item_property_name(item);
		if (lang::market::poe1::is_undroppable_unique(name)) {
			return;
		}

		const auto& base_type = item.at("baseType").get_ref<const nlohmann::json::string_t&>();
		uniques.add_item(base_type, lang::market::elementary_item{get_stash_item_price_data(item, rates), name});
	});
}

template <const char* FileStem>
void parse_and_fill_uniques(
	const json_file<FileStem>& json,
	lang::market::currency_exchange_rates rates,
	lang::market::poe1::unique_item_price_data& uniques,
	log::logger& logger)
{
	parse_and_fill_uniques(json.file_content, FileStem, rates, uniques, logger);
}

} // namespace

lang::market::poe1::item_price_data parse_item_price_data(const api_item_price_data& jsons, log::logger& logger)
{
	lang::market::poe1::item_price_data result;

	std::tie(result.rates, result.currency) = parse_exchange_currency(jsons.currency.file_content, logger);

	result.essences         = parse_exchange_items(jsons.essence,      result.rates, logger);
	result.vials            = parse_stash_items(   jsons.vial,         result.rates, logger);
	result.fossils          = parse_exchange_items(jsons.fossil,       result.rates, logger);
	result.oils             = parse_exchange_items(jsons.oil,          result.rates, logger);
	result.delirium_orbs    = parse_exchange_items(jsons.delirium_orb, result.rates, logger);
	result.artifacts        = parse_exchange_items(jsons.artifact,     result.rates, logger);
	result.tattoos          = parse_exchange_items(jsons.tattoo,       result.rates, logger);
	result.omens            = parse_exchange_items(jsons.omen,         result.rates, logger);
	result.runegrafts       = parse_exchange_items(jsons.runegraft,    result.rates, logger);

	result.resonators       = parse_exchange_items(jsons.resonator,    result.rates, logger);

	result.divination_cards = parse_exchange_items(jsons.divination_card, result.rates, logger);

	result.fragments        = parse_exchange_items(jsons.fragment,       result.rates, logger);
	result.scarabs          = parse_exchange_items(jsons.scarab,         result.rates, logger);
	result.allflame_embers  = parse_exchange_items(jsons.allflame_ember, result.rates, logger);

	result.invitations      = parse_stash_items(jsons.invitation, result.rates, logger);

	result.incubators       = parse_stash_items(jsons.incubator,  result.rates, logger);

	result.gems = parse_gems(jsons.skill_gem.file_content, result.rates, logger);

	result.bases = parse_bases(jsons.base_type.file_content, result.rates, logger);

	parse_and_fill_uniques(jsons.unique_armour,    result.rates, result.unique_eq,        logger);
	parse_and_fill_uniques(jsons.unique_weapon,    result.rates, result.unique_eq,        logger);
	parse_and_fill_uniques(jsons.unique_accessory, result.rates, result.unique_eq,        logger);
	parse_and_fill_uniques(jsons.unique_flask,     result.rates, result.unique_flasks,    logger);
	parse_and_fill_uniques(jsons.unique_tincture,  result.rates, result.unique_tinctures, logger);
	parse_and_fill_uniques(jsons.unique_jewel,     result.rates, result.unique_jewels,    logger);
	parse_and_fill_uniques(jsons.unique_map,       result.rates, result.unique_maps,      logger);
	parse_and_fill_uniques(jsons.unique_relic,     result.rates, result.unique_relics,    logger);

	// TODO "Misc Map Items"? Can all fragments be caught as "Map Fragments"?
	move_item("Valdo's Puzzle Box",               result.fragments, result.currency);
	move_item("Crescent Splinter",                result.fragments, result.currency);
	move_item("Simulacrum Splinter",              result.fragments, result.currency);
	move_item("Splinter of Xoph",                 result.fragments, result.currency);
	move_item("Splinter of Tul",                  result.fragments, result.currency);
	move_item("Splinter of Esh",                  result.fragments, result.currency);
	move_item("Splinter of Uul-Netol",            result.fragments, result.currency);
	move_item("Splinter of Chayula",              result.fragments, result.currency);
	move_item("Timeless Vaal Splinter",           result.fragments, result.currency);
	move_item("Timeless Karui Splinter",          result.fragments, result.currency);
	move_item("Timeless Eternal Empire Splinter", result.fragments, result.currency);
	move_item("Timeless Templar Splinter",        result.fragments, result.currency);
	move_item("Timeless Maraketh Splinter",       result.fragments, result.currency);

	/*
	 * not all jsons are being read but:
	 * - we do not care about non-unique maps - people filter them by tier
	 * - we do not care about beasts - they do not drop
	 */
	return result;
}

} // namespace poe1

namespace poe2 {

namespace {

void parse_uncut_gems(
	std::string_view json_str,
	lang::market::currency_exchange_rates rates,
	std::vector<lang::market::elementary_item>& uncut_skill_gems,
	std::vector<lang::market::elementary_item>& uncut_spirit_gems,
	log::logger& logger)
{
	std::vector<lang::market::elementary_item> uncut_gems =
		parse_exchange_items(nlohmann::json::parse(json_str), file_stem_uncut_gems, rates, logger);

	for (auto& uncut_gem: uncut_gems) {
		if (utility::contains(uncut_gem.name, "Skill"))
			uncut_skill_gems.push_back(std::move(uncut_gem));
		else if (utility::contains(uncut_gem.name, "Spirit"))
			uncut_spirit_gems.push_back(std::move(uncut_gem));
	}
}

} // namespace

lang::market::poe2::item_price_data parse_item_price_data(const api_item_price_data& jsons, log::logger& logger)
{
	lang::market::poe2::item_price_data result;

	std::tie(result.rates, result.currency) = parse_exchange_currency(jsons.currency.file_content, logger);

	result.fragments            = parse_exchange_items(jsons.fragments,            result.rates, logger);
	result.abyss_currency       = parse_exchange_items(jsons.abyss,                result.rates, logger);
	parse_uncut_gems(jsons.uncut_gems.file_content, result.rates, result.uncut_skill_gems, result.uncut_spirit_gems, logger);
	result.lineage_support_gems = parse_exchange_items(jsons.lineage_support_gems, result.rates, logger);
	result.essences             = parse_exchange_items(jsons.essences,             result.rates, logger);
	result.soul_cores           = parse_exchange_items(jsons.soul_cores,           result.rates, logger);
	result.idols                = parse_exchange_items(jsons.idols,                result.rates, logger);
	result.runes                = parse_exchange_items(jsons.runes,                result.rates, logger);
	result.omens                = parse_exchange_items(jsons.ritual,               result.rates, logger);
	result.expedition           = parse_exchange_items(jsons.expedition,           result.rates, logger);
	result.emotions             = parse_exchange_items(jsons.delirium,             result.rates, logger);
	result.catalysts            = parse_exchange_items(jsons.breach,               result.rates, logger);
	result.verisium             = parse_exchange_items(jsons.verisium,             result.rates, logger);

	move_item("Kulemak's Invitation", result.abyss_currency, result.fragments);

	move_item("Runic Splinter", result.fragments,  result.currency);
	move_item("Runic Splinter", result.expedition, result.currency);

	move_item("Petition Splinter", result.fragments, result.currency);
	move_item("Petition Splinter", result.omens,     result.currency);

	move_item("Simulacrum Splinter", result.fragments, result.currency);
	move_item("Simulacrum Splinter", result.emotions,  result.currency);

	move_item("Breach Splinter", result.fragments, result.currency);
	move_item("Breach Splinter", result.catalysts, result.currency);

	for (auto name : {"Amanamu's Gaze", "Tecrod's Gaze", "Kurgal's Gaze", "Ulaman's Gaze"})
		move_item(name, result.abyss_currency, result.runes);

	return result;
}

} // namespace poe2

}
