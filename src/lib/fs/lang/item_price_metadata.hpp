#pragma once

#include <fs/lang/data_source_type.hpp>
#include <fs/log/logger_fwd.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>

namespace fs::lang
{

struct item_price_metadata
{
	[[nodiscard]] bool save(const boost::filesystem::path& directory, fs::log::logger& logger) const;
	[[nodiscard]] bool load(const boost::filesystem::path& directory, fs::log::logger& logger);

	std::string league_name;
	data_source_type data_source;
	boost::posix_time::ptime download_date;
};

}
