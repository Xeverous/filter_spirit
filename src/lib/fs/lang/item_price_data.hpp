#pragma once

#include <fs/log/logger.hpp>
#include <fs/lang/data_source_type.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>

namespace fs::lang
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
		bool is_shaper,
		bool is_elder,
		bool is_crusader,
		bool is_redeemer,
		bool is_hunter,
		bool is_warlord)
	: elementary_item(std::move(ei))
	, item_level(item_level)
	, is_shaper(is_shaper)
	, is_elder(is_elder)
	, is_crusader(is_crusader)
	, is_redeemer(is_redeemer)
	, is_hunter(is_hunter)
	, is_warlord(is_warlord)
	{}

	int item_level;
	bool is_shaper;
	bool is_elder;
	bool is_crusader;
	bool is_redeemer;
	bool is_hunter;
	bool is_warlord;
};

bool is_undroppable_unique(std::string_view name) noexcept;

// unlinked uniques
struct unique_item_price_data
{
	void add_item(std::string base_type, elementary_item item_info);

	// maps base type name to unique item name
	// (only 1 unique on the given base)
	std::unordered_map<std::string, elementary_item> unambiguous;
	// maps base type name to unique item names
	// (contains multiple entries per base type)
	// the vector is never empty
	std::unordered_map<std::string, std::vector<elementary_item>> ambiguous;
};

struct item_price_metadata;

struct item_price_data
{
	[[nodiscard]] bool
	load_and_parse(
		const item_price_metadata& metadata,
		const std::string& directory_path,
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

	std::vector<elementary_item> catalysts;
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

struct item_price_metadata
{
	[[nodiscard]] bool save(const boost::filesystem::path& directory, fs::log::logger& logger) const;
	[[nodiscard]] bool load(const boost::filesystem::path& directory, fs::log::logger& logger);

	std::string league_name;
	data_source_type data_source;
	boost::posix_time::ptime download_date;
};

log::message_stream& operator<<(log::message_stream& stream, const item_price_metadata& ipm);

struct item_price_report
{
	item_price_data data;
	item_price_metadata metadata;
};

log::message_stream& operator<<(log::message_stream& stream, const item_price_report& ipr);

/**
 * Produce logs about differences in 2 item data sets
 *
 * both item data inputs must be sorted
 */
void compare_item_price_reports(
	const item_price_report& lhs,
	const item_price_report& rhs,
	log::logger& log);
}
