#include <fs/network/item_price_report.hpp>
#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/network/poe_ninja/parse_data.hpp>
#include <fs/network/poe_watch/download_data.hpp>
#include <fs/network/poe_watch/parse_data.hpp>
#include <fs/network/ggg/parse_data.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/async.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/dump_json.hpp>
#include <fs/utility/file.hpp>
#include <fs/version.hpp>

#include <nlohmann/json.hpp>

#include <utility>
#include <filesystem>

namespace {

constexpr auto cache_dir_path = "cache";
constexpr auto leagues_file_name = "leagues.json";

using namespace fs;
using namespace fs::network;

[[nodiscard]] bool
matches(
	const lang::market::item_price_metadata& metadata,
	const std::string& league,
	lang::data_source_type api)
{
	if (metadata.data_source != api)
		return false;

	if (metadata.league_name != league)
		return false;

	return true;
}

[[nodiscard]] bool
matches(
	const lang::market::item_price_metadata& metadata,
	const std::string& league,
	lang::data_source_type api,
	boost::posix_time::time_duration expiration_time)
{
	if (!matches(metadata, league, api))
		return false;

	const auto now = boost::posix_time::microsec_clock::universal_time();
	if (metadata.download_date + expiration_time < now)
		return false;

	return true;
}

std::string normalize_league_name(std::string_view league_name)
{
	std::string result;
	result.reserve(league_name.size());

	for (char c : league_name) {
		if ('0' <= c && c <= '9') {
			result.push_back(c);
			continue;
		}

		if ('a' <= c && c <= 'z') {
			result.push_back(c);
			continue;
		}

		if ('A' <= c && c <= 'Z') {
			result.push_back(c);
			continue;
		}

		if (c == ' ') {
			result.push_back('_');
			continue;
		}
	}

	return result;
}

std::string make_save_path(lang::data_source_type api, std::string_view league)
{
	std::string path = cache_dir_path;
	path.append("/");

	if (api == lang::data_source_type::poe_ninja)
		path += "ninja";
	else if (api == lang::data_source_type::poe_watch)
		path += "watch";
	else
		path += "unknown";

	// league name is normalized to avoid potential invalid characters in paths
	return path.append("_").append(normalize_league_name(league));
}

lang::market::item_price_report
load_item_price_report(
	item_price_report_cache& self,
	item_price_report_cache::metadata_save meta,
	log::logger& logger)
{
	std::optional<lang::market::item_price_report> report = lang::market::load_item_price_report(meta.path, logger);
	if (!report)
		throw std::runtime_error("failed to load item price report from disk");

	self.update_memory_cache(*report);
	return *report;

}

template <typename T>
void save_api_data(
	const T& api_data, // T must have save(dir, logger) overload
	const lang::market::item_price_metadata& metadata,
	const std::string& data_save_dir,
	log::logger& logger)
{
	if (!metadata.save(data_save_dir, logger)) {
		logger.error() << "Failed to save item price metadata.\n";
		return;
	}

	if (!api_data.save(data_save_dir, logger)) {
		logger.error() << "Failed to save item price data.\n";
		return;
	}

	logger.info() << "Item price data successfully saved.\n";
}

lang::market::item_price_report
download_and_parse_ninja(
	item_price_report_cache& self,
	std::string league,
	download_settings settings,
	download_info* info,
	log::logger& logger)
{
	poe_ninja::api_item_price_data api_data = poe_ninja::download_item_price_data(league, settings, info, logger);

	lang::market::item_price_report report;
	report.metadata.data_source = lang::data_source_type::poe_ninja;
	report.metadata.league_name = league;
	report.metadata.download_date = boost::posix_time::microsec_clock::universal_time();

	std::string save_path = make_save_path(lang::data_source_type::poe_ninja, league);
	save_api_data(api_data, report.metadata, save_path, logger);

	report.data = poe_ninja::parse_item_price_data(api_data, logger);

	self.update_memory_cache(report);
	self.update_disk_cache({report.metadata, save_path, version::current()});
	self.update_cache_file_on_disk(logger);
	return report;

}

lang::market::item_price_report
download_and_parse_watch(
	item_price_report_cache& self,
	std::string league,
	download_settings settings,
	download_info* info,
	log::logger& logger)
{
	poe_watch::api_item_price_data api_data = poe_watch::download_item_price_data(league, settings, info, logger);

	lang::market::item_price_report report;
	report.metadata.data_source = lang::data_source_type::poe_ninja;
	report.metadata.league_name = league;
	report.metadata.download_date = boost::posix_time::microsec_clock::universal_time();

	std::string save_path = make_save_path(lang::data_source_type::poe_watch, league);
	save_api_data(api_data, report.metadata, save_path, logger);

	report.data = poe_watch::parse_item_price_data(api_data, logger);

	self.update_memory_cache(report);
	self.update_disk_cache({report.metadata, save_path, version::current()});
	self.update_cache_file_on_disk(logger);
	return report;
}

} // namespace

namespace fs::network {

std::vector<lang::league> load_leagues_from_disk(log::logger& logger)
{
	const auto path = std::filesystem::path(cache_dir_path) / leagues_file_name;
	std::error_code ec;
	std::string file_content = utility::load_file(path, ec);
	if (ec)
		return {}; // missing cache file => not an error => return empty vector

	try {
		return ggg::parse_league_info({std::move(file_content)});
	}
	catch (const std::exception& e) {
		logger.error() << "While loading " << path.generic_string() << ": " << e.what()
			<< ". Cache is probably corrupted, removing leagues cache file.\n";
		std::filesystem::remove(path);
		return {};
	}
}

void update_leagues_on_disk(const ggg::api_league_data& api_data, log::logger& logger)
{
	const auto path = std::filesystem::path(cache_dir_path) / leagues_file_name;

	if (!(
		utility::create_directories(cache_dir_path, logger) &&
		utility::save_file(path, api_data.leagues_json, logger)
	)) {
		logger.error() << "Failed to save leagues cache file.\n";
	}
}

lang::market::item_price_report
item_price_report_cache::get_report(
	std::string league,
	lang::data_source_type api,
	boost::posix_time::time_duration expiration_time,
	download_settings settings,
	download_info* info,
	log::logger& logger)
{
	if (api == lang::data_source_type::none) {
		return lang::market::item_price_report();
	}

	if (std::optional<lang::market::item_price_report> report = find_in_memory_cache(league, api, expiration_time); report) {
		return *report;
	}

	if (std::optional<metadata_save> metadata = find_in_disk_cache(league, api, expiration_time); metadata) {
		return load_item_price_report(*this, std::move(*metadata), logger);
	}

	if (api == lang::data_source_type::poe_ninja) {
		return download_and_parse_ninja(*this, std::move(league), std::move(settings), info, logger);
	}
	else /* if (api == lang::data_source_type::poe_watch) */ {
		FS_ASSERT(api == lang::data_source_type::poe_watch);
		return download_and_parse_watch(*this, std::move(league), std::move(settings), info, logger);
	}
}

[[nodiscard]] std::optional<lang::market::item_price_report>
item_price_report_cache::find_in_memory_cache(
	const std::string& league,
	lang::data_source_type api,
	boost::posix_time::time_duration expiration_time) const
{
	auto _ = std::lock_guard<std::mutex>(_memory_cache_mutex);

	for (const lang::market::item_price_report& rep : _memory_cache) {
		if (matches(rep.metadata, league, api, expiration_time))
			return rep;
	}

	return std::nullopt;
}

[[nodiscard]] std::optional<item_price_report_cache::metadata_save>
item_price_report_cache::find_in_disk_cache(
	const std::string& league,
	lang::data_source_type api,
	boost::posix_time::time_duration expiration_time) const
{
	auto _ = std::lock_guard<std::mutex>(_disk_cache_mutex);

	for (const metadata_save& metadata : _disk_cache) {
		if (matches(metadata.metadata, league, api, expiration_time))
			return metadata;
	}

	return std::nullopt;
}

void item_price_report_cache::update_disk_cache(metadata_save newer)
{
	auto _ = std::lock_guard<std::mutex>(_disk_cache_mutex);

	for (metadata_save& metadata : _disk_cache) {
		if (matches(metadata.metadata, newer.metadata.league_name, newer.metadata.data_source)) {
			metadata = std::move(newer);
			return;
		}
	}

	_disk_cache.push_back(std::move(newer));
}

void item_price_report_cache::update_memory_cache(lang::market::item_price_report newer)
{
	auto _ = std::lock_guard<std::mutex>(_memory_cache_mutex);

	for (lang::market::item_price_report& rep : _memory_cache) {
		if (matches(rep.metadata, newer.metadata.league_name, newer.metadata.data_source)) {
			rep = std::move(newer);
			return;
		}
	}

	_memory_cache.push_back(std::move(newer));
}

constexpr auto field_path = "path";
constexpr auto field_fs_version = "fs_version";

constexpr auto cache_metadata_path = "cache/metadata.json";

bool item_price_report_cache::update_cache_file_on_disk(log::logger& logger) const
{
	std::vector<metadata_save> saves = [this]() {
		auto _ = std::lock_guard<std::mutex>(_disk_cache_mutex);
		return _disk_cache;
	}();

	auto json = nlohmann::json::array();
	for (const metadata_save& save : saves) {
		nlohmann::json object = to_json(save.metadata);
		object[field_path] = save.path.generic_string();
		const auto v = fs::version::current();
		object[field_fs_version] = nlohmann::json::array({v.major, v.minor, v.patch});

		json.push_back(std::move(object));
	}

	return utility::save_file(cache_metadata_path, utility::dump_json(json), logger);
}

bool item_price_report_cache::load_cache_file_from_disk(log::logger& logger)
{
	if (!std::filesystem::exists(cache_metadata_path)) {
		// no recent cache file - behave as if it was empty and return success immediately
		return true;
	}

	// make a separate vector instance to implement strong exception guuarantee
	std::vector<metadata_save> saves;

	std::optional<std::string> file = utility::load_file(cache_metadata_path, logger);
	if (!file)
		return false;

	auto json = nlohmann::json::parse(*file);
	for (const auto& obj : json) {
		std::optional<lang::market::item_price_metadata> meta = lang::market::from_json(obj, logger);
		if (!meta) {
			logger.error() << "Failed to parse metadata.\n";
			return false;
		}

		metadata_save save;
		save.metadata = std::move(*meta);
		save.path = obj.at(field_path).get<std::string>();

		auto& version_arr = obj.at(field_fs_version).get_ref<const nlohmann::json::array_t&>();
		save.fs_version.major = version_arr.at(0).get<int>();
		save.fs_version.minor = version_arr.at(1).get<int>();
		save.fs_version.patch = version_arr.at(2).get<int>();

		saves.push_back(std::move(save));
	}

	{
		auto _ = std::lock_guard<std::mutex>(_disk_cache_mutex);
		_disk_cache = std::move(saves);
	}

	return true;
}

}
