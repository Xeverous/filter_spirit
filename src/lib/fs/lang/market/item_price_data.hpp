#pragma once

#include <fs/log/logger.hpp>
#include <fs/lang/data_source_type.hpp>
#include <fs/lang/influence_info.hpp>

#include <nlohmann/json.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include <filesystem>
#include <optional>

namespace fs::lang::market
{

struct price_data
{
	double chaos_value;
	bool is_low_confidence;
};

struct elementary_item
{
	price_data price;
	std::string name;
};

struct divination_card : elementary_item
{
	divination_card(elementary_item ei, int stack_size)
	: elementary_item(std::move(ei)), stack_size(stack_size) {}

	divination_card(price_data price, std::string name, int stack_size)
	: elementary_item{price, std::move(name)}, stack_size(stack_size) {}

	int stack_size;
};

struct gem : elementary_item
{
	gem(elementary_item ei, int level, int quality, bool is_corrupted)
	: elementary_item(std::move(ei)), level(level), quality(quality), is_corrupted(is_corrupted) {}

	int level;
	int quality;
	bool is_corrupted;
};

struct base : elementary_item
{
	base(
		elementary_item ei,
		int item_level,
		influence_info influence)
	: elementary_item(std::move(ei))
	, item_level(item_level)
	, influence(influence)
	{}

	int item_level;
	influence_info influence;
};

bool is_undroppable_unique(std::string_view name) noexcept;
bool is_drop_disabled_prophecy(std::string_view name) noexcept;

// unlinked uniques
struct unique_item_price_data
{
	void add_item(std::string base_type, elementary_item item_info);

	// maps base type name to unique item name
	// (only 1 unique on the given base)
	using unambiguous_container_type = std::unordered_map<std::string, elementary_item>;
	unambiguous_container_type unambiguous;
	// maps base type name to unique item names
	// (contains multiple entries per base type)
	// the vector should never have size 0 or 1
	using ambiguous_container_type = std::unordered_map<std::string, std::vector<elementary_item>>;
	ambiguous_container_type ambiguous;
};

struct item_price_metadata;

struct item_price_data
{
	[[nodiscard]] bool
	load_and_parse(
		const item_price_metadata& metadata,
		const std::filesystem::path& directory_path,
		log::logger& logger);

	/**
	 * @brief sort all non-unique item categories by name
	 *
	 * Purposes:
	 * - faster searching of the given item (binary search)
	 * - efficient comparison of 2 item_price_data instances
	 */
	void sort();

	std::vector<divination_card> divination_cards;

	std::vector<elementary_item> currency;
	std::vector<elementary_item> fragments;
	std::vector<elementary_item> delirium_orbs;
	std::vector<elementary_item> vials;
	std::vector<elementary_item> oils;
	std::vector<elementary_item> incubators;
	std::vector<elementary_item> essences;
	std::vector<elementary_item> fossils;
	std::vector<elementary_item> prophecies;
	std::vector<elementary_item> resonators;
	std::vector<elementary_item> scarabs;
	std::vector<elementary_item> helmet_enchants;

	std::vector<gem> gems;

	std::vector<base> bases;

	unique_item_price_data unique_eq; // jewellery, body parts, weapons
	unique_item_price_data unique_flasks;
	unique_item_price_data unique_jewels;
	unique_item_price_data unique_maps;
};


log::message_stream& operator<<(log::message_stream& stream, const item_price_data& ipd);

nlohmann::json to_json(const item_price_metadata& metadata);
std::optional<item_price_metadata> from_json(const nlohmann::json& object, log::logger& logger);

struct item_price_metadata
{
	[[nodiscard]] bool save(const std::filesystem::path& directory, log::logger& logger) const;
	[[nodiscard]] bool load(const std::filesystem::path& directory, log::logger& logger);

	std::string league_name = "(none)";
	data_source_type data_source = data_source_type::none;
	boost::posix_time::ptime download_date = boost::posix_time::ptime(boost::posix_time::not_a_date_time);
};

log::message_stream& operator<<(log::message_stream& stream, const item_price_metadata& ipm);

struct item_price_report
{
	item_price_data data;
	item_price_metadata metadata;
};

log::message_stream& operator<<(log::message_stream& stream, const item_price_report& ipr);

std::optional<item_price_report>
load_item_price_report(
	const std::filesystem::path& directory,
	log::logger& logger);

/**
 * Produce logs about differences in 2 item data sets
 *
 * both item data inputs must be sorted
 */
void compare_item_price_reports(
	const item_price_report& lhs,
	const item_price_report& rhs,
	log::logger& logger);
}
