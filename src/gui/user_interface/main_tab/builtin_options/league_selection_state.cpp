#include "user_interface/main_tab/builtin_options/league_selection_state.hpp"
#include "ui_utils.hpp"

#include <fs/network/ggg/parse_data.hpp>
#include <fs/utility/async.hpp>

namespace el = cycfi::elements;

namespace {

auto make_league_selection_menu(const std::vector<std::string>& leagues, std::function<void(std::string_view)> on_select)
{
	return el::selection_menu(std::move(on_select), leagues).first;
}

}

std::function<void(std::string_view)> league_selection_state::make_league_selection_callback()
{
	return [this](std::string_view selected) {
		_selected_league = selected;
		_inserter.push_event(events::league_selection_changed{});
	};
}

void league_selection_state::update(fs::log::logger& logger)
{
	if (!_download_running)
		return;

	if (fs::utility::is_ready(_leagues_future)) {
		_download_running = false;

		std::vector<fs::lang::league> leagues = fs::network::ggg::parse_league_info(_leagues_future.get());

		_available_leagues.clear();
		for (const auto& league : leagues)
			_available_leagues.push_back(std::move(league.name));

		_league_selection_menu->subject(make_league_selection_menu(_available_leagues, make_league_selection_callback()));
		_league_selection_refresh_element->select(1); // swicth to refresh button
		logger.info() << _available_leagues.size() << " non-SSF leagues are available to download prices from\n";
	}
	else {
		const auto xfer_info = _league_download_info.xfer_info.load(std::memory_order_relaxed);
		_league_selection_refresh_progress_bar->value(
			calculate_progress(xfer_info.bytes_downloaded_so_far, xfer_info.expected_download_size));
	}
}

void league_selection_state::refresh_available_leagues(fs::network::network_settings ns, fs::log::logger& logger)
{
	if (_download_running)
		return; // ignore request if download is already in progress

	_league_selection_refresh_element->select(0); // switch to progress bar
	_league_selection_refresh_progress_bar->value(0);

	_leagues_future = fs::network::ggg::async_download_leagues(ns, &_league_download_info, logger);
	_download_running = true;
}

std::shared_ptr<el::element> league_selection_state::make_ui()
{
	_league_selection_menu = el::share(make_proxy(make_league_selection_menu(_available_leagues, make_league_selection_callback())));
	_league_selection_refresh_progress_bar = el::share(make_progress_bar());
	_league_selection_refresh_element = el::share(el::deck(
		el::hold(_league_selection_refresh_progress_bar),
		make_refresh_button([this](bool) {
			_inserter.push_event(events::refresh_available_leagues{});
		})
	));
	_league_selection_refresh_element->select(1); // start with refresh button

	return el::share(el::htile(
		el::hold(_league_selection_menu),
		el::hold(_league_selection_refresh_element)
	));
}
