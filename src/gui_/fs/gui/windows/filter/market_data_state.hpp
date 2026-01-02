#pragma once

#include <fs/lang/market/item_price_data.hpp>
#include <fs/lang/league.hpp>
#include <fs/network/download.hpp>
#include <fs/network/item_price_report.hpp>

#include <utility>
#include <future>
#include <memory>

namespace fs::gui {

class network_settings;
class spirit_filter_state_mediator;

class market_data_state
{
public:
	market_data_state(std::vector<lang::league> available_leagues)
	: _available_leagues(std::move(available_leagues))
	{
	}

	void draw_interface(const network_settings& settings, network::cache& cache, spirit_filter_state_mediator& mediator);

	const lang::market::item_price_report& price_report() const
	{
		return _price_report;
	}

private:
	void on_league_change(
		const network_settings& settings,
		network::item_price_report_cache& cache,
		spirit_filter_state_mediator& mediator)
	{
		refresh_item_price_report(settings, cache, mediator);
	}

	void on_api_change(
		const network_settings& settings,
		network::item_price_report_cache& cache,
		spirit_filter_state_mediator& mediator)
	{
		refresh_item_price_report(settings, cache, mediator);
	}

	void refresh_item_price_report(
		const network_settings& settings,
		network::item_price_report_cache& cache,
		spirit_filter_state_mediator& mediator);
	void refresh_available_leagues(const network_settings& settings, spirit_filter_state_mediator& mediator);

	void check_downloads(const network_settings& settings, network::cache& cache, spirit_filter_state_mediator& mediator);

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
