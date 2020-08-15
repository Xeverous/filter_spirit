#pragma once

#include "event.hpp"
#include "user_interface/main_tab/builtin_options/api_selection_state.hpp"
#include "user_interface/main_tab/builtin_options/market_data_state.hpp"
#include "user_interface/main_tab/builtin_options/league_selection_state.hpp"
#include "user_interface/generic/integer_input_box.hpp"

#include <fs/log/logger.hpp>
#include <fs/lang/limits.hpp>

#include <elements/element/element.hpp>

#include <memory>

class builtin_options_state
{
public:
	builtin_options_state(event_inserter inserter)
	: _api_selection(inserter)
	, _league_selection(inserter)
	, _market_data(inserter)
	, _override_opacity_option(0, 255)
	, _override_volume_option(fs::lang::limits::min_filter_volume, fs::lang::limits::max_filter_volume)
	{
	}

	void update(fs::log::logger& logger)
	{
		_league_selection.update(logger);
		_market_data.update();
	}

	std::shared_ptr<cycfi::elements::element> make_ui();

	auto& api_selection() { return _api_selection; }
	auto& league_selection() { return _league_selection; }
	auto& market_data() { return _market_data; }
	auto& override_opacity_option() { return _override_opacity_option; }
	auto& override_volume_option() { return _override_volume_option; }

private:
	api_selection_state _api_selection;
	league_selection_state _league_selection;
	market_data_state _market_data;
	integer_input_box _override_opacity_option;
	integer_input_box _override_volume_option;
};
