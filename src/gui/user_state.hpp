#pragma once

#include "gui_logger.hpp"

#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/log/logger.hpp>

#include <string>

struct filter_state
{
	std::optional<std::string> template_source;
	std::optional<fs::lang::spirit_item_filter> spirit_filter;
	std::optional<fs::lang::item_price_report> price_report;
};

struct generation_settings
{
	std::string filter_template_path;
	// std::string output_directory;
	// std::string generated_files_name_scheme;
	fs::lang::data_source_type data_source;
	std::string league;
};

struct networking_settings
{
	std::string proxy;
	std::string certificases_bundle_path;
	int timeout;
	bool ssl_verify_peer;
	bool ssl_verify_host;
};

struct program_settings
{
	networking_settings networking;
};

struct user_state
{
	[[nodiscard]] bool save() const;
	[[nodiscard]] bool load();

	void load_filter_template();
	void parse_filter_template();

	filter_state filter;
	generation_settings generation;
	program_settings program;
	gui_logger logger;
};
