#pragma once

#include "event.hpp"

#include <fs/network/ggg/download_data.hpp>
#include <fs/log/logger.hpp>

#include <elements/element.hpp>

#include <memory>
#include <string>
#include <vector>
#include <queue>

class league_selection_state
{
public:
	league_selection_state(event_inserter inserter)
	: _inserter(inserter)
	{
		_inserter.push_event(events::refresh_available_leagues{});
	}

	std::shared_ptr<cycfi::elements::element> make_ui();

	void refresh_available_leagues(fs::network::download_settings ds, fs::log::logger& logger);
	void update(fs::log::logger& logger);

	const std::string& selected_league() const
	{
		return _selected_league;
	}

	const std::vector<std::string>& available_leagues() const
	{
		return _available_leagues;
	}

private:
	std::function<void(std::string_view)> make_league_selection_callback();

	event_inserter _inserter;

	// actual state
	std::string _selected_league;
	std::vector<std::string> _available_leagues;

	// download state
	bool _download_running = false;
	fs::network::download_info _league_download_info;
	std::future<fs::network::ggg::api_league_data> _leagues_future;

	// UI elements
	std::shared_ptr<cycfi::elements::deck_element> _league_selection_refresh_element;
	std::shared_ptr<cycfi::elements::progress_bar_base> _league_selection_refresh_progress_bar;
	std::shared_ptr<cycfi::elements::proxy<cycfi::elements::basic_menu>> _league_selection_menu;
};
