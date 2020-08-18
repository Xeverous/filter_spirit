#pragma once

#include "event.hpp"

#include <fs/lang/market/item_price_data.hpp>
#include <fs/network/item_price_report.hpp>

#include <elements/element.hpp>

#include <memory>

class market_data_state
{
public:
	market_data_state(event_inserter inserter)
	: _inserter(inserter)
	{
		make_ui();
	}

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

	void update();

	void refresh_market_data(
		fs::lang::data_source_type api,
		std::string league,
		boost::posix_time::time_duration max_age,
		fs::network::download_settings ds,
		fs::log::logger& logger);

	const auto& price_report() const { return _price_report; }

private:
	void make_ui();
	void refresh_status_label();

	event_inserter _inserter;

	// actual state
	fs::lang::market::item_price_report _price_report;

	// download state
	bool _download_running = false;
	fs::network::download_info _download_info;
	std::future<fs::lang::market::item_price_report> _price_report_future;
	fs::network::item_price_report_cache _price_report_cache;

	// UI
	std::shared_ptr<cycfi::elements::element> _root_element;
	std::shared_ptr<cycfi::elements::deck_element> _market_data_refresh_element;
	std::shared_ptr<cycfi::elements::progress_bar_base> _market_data_refresh_progress_bar;
	std::shared_ptr<cycfi::elements::label> _market_data_status_label;
};
