#pragma once

#include <fs/lang/item_price_data.hpp>
#include <fs/network/download.hpp>
#include <fs/log/logger.hpp>
#include <fs/version.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <future>
#include <string>
#include <filesystem>
#include <optional>

namespace fs::network {

class item_price_report_cache
{
public:
	[[nodiscard]] std::future<lang::item_price_report>
	async_get_report(
		std::string league,
		lang::data_source_type api,
		boost::posix_time::time_duration expiration_time,
		network_settings settings,
		download_info* info,
		log::logger& logger);


	struct metadata_save
	{
		lang::item_price_metadata metadata;
		std::filesystem::path path;
		version::version_triplet fs_version;
	};

	void update_disk_cache(metadata_save metadata);
	void update_memory_cache(lang::item_price_report report);

	bool update_cache_file_on_disk(log::logger& logger) const;
	bool load_cache_file_from_disk(log::logger& logger);

private:
	[[nodiscard]] std::optional<lang::item_price_report>
	find_in_memory_cache(
		const std::string& league,
		lang::data_source_type api,
		boost::posix_time::time_duration expiration_time) const;

	[[nodiscard]] std::optional<metadata_save>
	find_in_disk_cache(
		const std::string& league,
		lang::data_source_type api,
		boost::posix_time::time_duration expiration_time) const;

	mutable std::mutex _disk_cache_mutex;
	mutable std::mutex _memory_cache_mutex;
	std::vector<metadata_save> _disk_cache;
	std::vector<lang::item_price_report> _memory_cache;
};

}
