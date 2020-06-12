#pragma once

#include "gui_logger.hpp"

#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/network/download.hpp>
#include <fs/network/ggg/download_data.hpp>
#include <fs/log/logger.hpp>

#include <string>
#include <future>

struct filter_state
{
	std::optional<std::string> template_source;
	std::optional<fs::lang::spirit_item_filter> spirit_filter;
	std::optional<fs::lang::item_price_report> price_report;
};

struct league_selection_state
{
	std::string selected_league;
	fs::network::download_info league_download_info;
	std::future<fs::network::ggg::api_league_data> leagues_future;
};

struct generation_settings
{
	std::string filter_template_path;
	fs::lang::data_source_type data_source;
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

	void load_filter_template();
	void parse_filter_template();
	void download_available_leagues();

	filter_state filter;
	generation_settings generation;
	program_settings program;
	gui_logger logger;
};
