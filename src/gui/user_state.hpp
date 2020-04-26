#pragma once

#include "gui_logger.hpp"

#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/network/download.hpp>
#include <fs/network/ggg/download_data.hpp>
#include <fs/network/item_price_report.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>
#include <string_view>
#include <optional>
#include <future>
#include <functional>

struct filter_state
{
	std::optional<std::string> template_source;
	std::optional<fs::lang::spirit_item_filter> spirit_filter;
	std::optional<fs::lang::item_filter> real_filter;
};

struct league_selection_state
{
	// actual state
	std::string selected_league;
	std::vector<std::string> available_leagues;

	// download state
	bool download_running = false;
	fs::network::download_info league_download_info;
	std::future<fs::network::ggg::api_league_data> leagues_future;

	// callbacks
	std::function<void()> refresh_available_leagues;
	std::function<void()> update;
};

struct item_price_report_state
{
	// actual state
	fs::lang::data_source_type selected_api = fs::lang::data_source_type::poe_ninja;
	fs::lang::item_price_report price_report;

	// download state
	bool download_running = false;
	fs::network::download_info download_info;
	std::future<fs::lang::item_price_report> data_future;
	fs::network::item_price_report_cache cache;

	// callbacks
	std::function<void(boost::posix_time::time_duration)> refresh_item_price_data;
	std::function<void()> update;
};

struct generation_settings
{
	std::string filter_template_path;
	item_price_report_state item_price_data_state;
	league_selection_state league_state;
};

struct program_settings
{
	fs::network::network_settings networking;
};

struct user_state
{
	[[nodiscard]] bool save() const;
	[[nodiscard]] bool load();

	void update(); // call once per frame

	void load_filter_template();
	void parse_filter_template();
	void recompute_real_filter();
	void regenerate_loot_preview();

	void refresh_available_leagues();

	filter_state filter;
	generation_settings generation;
	program_settings program;
	gui_logger logger;
};
