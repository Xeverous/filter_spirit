#pragma once

#include <fs/gui/windows/filter/spirit_filter_state_mediator_base.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/network/download.hpp>
#include <fs/network/ggg/api_data.hpp>

#include <future>
#include <memory>

namespace fs::gui {

class application;

class market_data_state
{
public:
	void draw_interface(application& app, spirit_filter_state_mediator_base& mediator);

	const lang::market::item_price_report& price_report() const
	{
		return _price_report;
	}

private:
	void on_league_change(application& app, spirit_filter_state_mediator_base& mediator) { refresh_item_price_report(app, mediator); }
	void on_api_change   (application& app, spirit_filter_state_mediator_base& mediator) { refresh_item_price_report(app, mediator); }

	void refresh_item_price_report(application& app, spirit_filter_state_mediator_base& mediator);

	lang::data_source_type _selected_api = lang::data_source_type::poe_ninja;

	std::optional<std::string> _selected_league;
	lang::market::item_price_report _price_report;

	bool _league_download_running = false;
	std::shared_ptr<network::download_info> _league_download_info;
	std::future<network::ggg::api_league_data> _leagues_future;

	bool _price_report_download_running = false;
	std::shared_ptr<network::download_info> _price_report_download_info;
	std::future<lang::market::item_price_report> _price_report_future;
};

}
