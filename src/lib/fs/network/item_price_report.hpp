#pragma once

#include <fs/lang/market/item_price_data.hpp>
#include <fs/lang/league.hpp>
#include <fs/network/download.hpp>
#include <fs/network/ggg/api_data.hpp>
#include <fs/log/logger.hpp>
#include <fs/version.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>
#include <filesystem>
#include <optional>

namespace fs::network {

std::vector<lang::league> load_leagues_from_disk(log::logger& logger);
void update_leagues_on_disk(const ggg::api_league_data& api_data, log::logger& logger);

class leagues_cache
{
public:
	std::vector<lang::league> get_leagues() const
	{
		auto _ = std::lock_guard<std::mutex>(_mutex);
		return _leagues;
	}

	void set_leagues(const std::vector<lang::league>& leagues)
	{
		auto _ = std::lock_guard<std::mutex>(_mutex);
		_leagues = leagues;
	}

	void set_leagues(std::vector<lang::league>&& leagues)
	{
		auto _ = std::lock_guard<std::mutex>(_mutex);
		_leagues = std::move(leagues);
	}

private:
	std::vector<lang::league> _leagues;
	mutable std::mutex _mutex;
};

class item_price_report_cache
{
public:
	/*
	 * This function may run a long time
	 * logger and info (if non-null) must live for the call duration.
	 */
	[[nodiscard]] lang::market::item_price_report
	get_report(
		std::string league,
		lang::data_source_type api,
		boost::posix_time::time_duration expiration_time,
		download_settings settings,
		download_info* info,
		log::logger& logger);

	struct metadata_save
	{
		lang::market::item_price_metadata metadata;
		std::filesystem::path path;
		version::version_triplet fs_version;
	};

	void update_disk_cache(metadata_save metadata);
	void update_memory_cache(lang::market::item_price_report report);

	bool update_cache_file_on_disk(log::logger& logger) const;
	bool load_cache_file_from_disk(log::logger& logger);

	[[nodiscard]] std::optional<lang::market::item_price_report>
	find_in_memory_cache(
		const std::string& league,
		lang::data_source_type api,
		boost::posix_time::time_duration expiration_time) const;

private:
	[[nodiscard]] std::optional<metadata_save>
	find_in_disk_cache(
		const std::string& league,
		lang::data_source_type api,
		boost::posix_time::time_duration expiration_time) const;

	mutable std::mutex _disk_cache_mutex;
	mutable std::mutex _memory_cache_mutex;
	std::vector<metadata_save> _disk_cache;
	std::vector<lang::market::item_price_report> _memory_cache;
};

struct cache
{
	leagues_cache leagues;
	item_price_report_cache item_price_reports;
};

}
