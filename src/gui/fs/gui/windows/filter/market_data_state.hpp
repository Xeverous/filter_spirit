#pragma once

#include <fs/lang/market/item_price_data.hpp>
#include <fs/lang/league.hpp>
#include <fs/network/download.hpp>

#include <utility>
#include <future>
#include <memory>

namespace fs::gui {

class application;
class spirit_filter_state_mediator;

class market_data_state
{
public:
	market_data_state(std::vector<lang::league> available_leagues)
	: _available_leagues(std::move(available_leagues))
	{
	}

	void draw_interface(application& app, spirit_filter_state_mediator& mediator);

	const lang::market::item_price_report& price_report() const
	{
		return _price_report;
	}

private:
	void on_league_change(application& app, spirit_filter_state_mediator& mediator) { refresh_item_price_report(app, mediator); }
	void on_api_change   (application& app, spirit_filter_state_mediator& mediator) { refresh_item_price_report(app, mediator); }

	void refresh_item_price_report(application& app, spirit_filter_state_mediator& mediator);
	void refresh_available_leagues(application& app, spirit_filter_state_mediator& mediator);

	void check_downloads(application& app, log::logger& logger);

	lang::data_source_type _selected_api = lang::data_source_type::poe_ninja;

	std::optional<std::string> _selected_league;
	std::vector<lang::league> _available_leagues;
	lang::market::item_price_report _price_report;

	bool _leagues_download_running = false;
	std::shared_ptr<network::download_info> _leagues_download_info;
	std::future<std::vector<lang::league>> _leagues_future;

	bool _price_report_download_running = false;
	std::shared_ptr<network::download_info> _price_report_download_info;
	std::future<lang::market::item_price_report> _price_report_future;
};

}
