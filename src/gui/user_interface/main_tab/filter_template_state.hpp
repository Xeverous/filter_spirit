#pragma once

#include "event.hpp"

#include <fs/generator/settings.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/log/logger.hpp>

#include <elements/element.hpp>

#include <optional>
#include <string>
#include <memory>

class filter_template_state
{
public:
	filter_template_state(event_inserter inserter)
	: _inserter(inserter)
	{
	}

	std::shared_ptr<cycfi::elements::element> make_ui(cycfi::elements::host_window_handle window);

	void load_filter_template(fs::log::logger& logger);
	void parse_filter_template(fs::generator::settings st, fs::log::logger& logger);
	void recompute_real_filter(const fs::lang::market::item_price_report& report, fs::log::logger& logger);

private:
	event_inserter _inserter;

	std::optional<std::string> _filter_template_path;
	std::optional<std::string> _filter_template_source;
	std::optional<fs::lang::spirit_item_filter> _spirit_filter;
	std::optional<fs::lang::item_filter> _real_filter;

	std::shared_ptr<cycfi::elements::basic_label> _label_filter_template_path;
};
