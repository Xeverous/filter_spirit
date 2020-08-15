#include "user_interface/main_tab/builtin_options/market_data_state.hpp"
#include "ui_utils.hpp"

#include <fs/utility/async.hpp>

#include <elements/element.hpp>

#include <utility>

namespace el = cycfi::elements;

void market_data_state::refresh_status_label()
{
	if (_price_report.metadata.data_source == fs::lang::data_source_type::none) {
		_market_data_refresh_element->select(2); // switch to no-refresh label
		_market_data_status_label->set_text("(no data)");
		return;
	}

	_market_data_refresh_element->select(1); // switch to refresh button

	const auto& download_date = _price_report.metadata.download_date;
	if (!download_date.is_special()) {
		const auto now = boost::posix_time::microsec_clock::universal_time();
		const auto time_diff = now - download_date;
		_market_data_status_label->set_text("using cached data from " + to_string(time_diff) + " ago");
	}
}

void market_data_state::update()
{
	if (_download_running) {
		if (fs::utility::is_ready(_price_report_future)) {
			_price_report = _price_report_future.get();
			_download_running = false;

			_inserter.push_event(events::price_report_changed{});

			refresh_status_label();
		}
		else {
			const auto requests_complete = _download_info.requests_complete.load(std::memory_order_relaxed);
			const auto requests_total    = _download_info.requests_total.load(std::memory_order_relaxed);
			_market_data_refresh_progress_bar->value(calculate_progress(requests_complete, requests_total));
		}
	}
	else {
		// no update running - just update UI label
		refresh_status_label();
	}
}

void market_data_state::refresh_market_data(
	fs::lang::data_source_type api,
	std::string league,
	boost::posix_time::time_duration max_age,
	fs::network::download_settings ds,
	fs::log::logger& logger)
{
	if (league.empty())
		return;

	_price_report_future = _price_report_cache.async_get_report(
		std::move(league), api, max_age, std::move(ds), &_download_info, logger);
	_download_running = true;

	_market_data_status_label->set_text("downloading...");
	_market_data_refresh_element->select(0); // switch to progress bar
	_market_data_refresh_progress_bar->value(0);
}

std::shared_ptr<cycfi::elements::element> market_data_state::make_ui()
{
	_market_data_status_label = el::share(el::label(""));
	_market_data_refresh_progress_bar = el::share(make_progress_bar());

	_market_data_refresh_element = el::share(el::deck(
		el::hold(_market_data_refresh_progress_bar),
		make_refresh_button([this](bool) {
			_inserter.push_event(events::refresh_market_data{});
		}),
		el::align_middle(el::label("(nothing to refresh)"))
	));
	_market_data_refresh_element->select(1);

	return el::share(el::htile(
		el::hold(_market_data_status_label),
		el::hold(_market_data_refresh_element)
	));
}
