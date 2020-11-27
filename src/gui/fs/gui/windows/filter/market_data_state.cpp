#include <fs/gui/windows/filter/market_data_state.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

#include <initializer_list>

namespace fs::gui {

void market_data_state::refresh_item_price_report(application& app, spirit_filter_state_mediator_base& mediator)
{
	if (!_selected_league) {
		_price_report = {};
		mediator.on_price_report_change(_price_report);
		return;
	}

	auto& cache = app.price_report_cache();
	const auto max_market_data_age = app.network_settings().max_market_data_age();
	std::optional<lang::market::item_price_report> opt_price_report = cache.find_in_memory_cache(
		*_selected_league, _selected_api, max_market_data_age);
	if (opt_price_report) {
		_price_report = std::move(*opt_price_report);
		mediator.on_price_report_change(_price_report);
		return;
	}

	if (!_price_report_download_info)
		_price_report_download_info = std::make_shared<network::download_info>();

	_price_report_future = std::async(
		std::launch::async, [
			&cache,
			league = *_selected_league,
			api = _selected_api,
			max_age = max_market_data_age,
			settings = app.network_settings().download_settings(),
			download_info = _price_report_download_info,
			logger = mediator.share_logger()]()
		{
			return cache.get_report(league, api, max_age, settings, download_info.get(), *logger);
		}
	);
	_price_report_download_running = true;
}

void market_data_state::draw_interface(application& app, spirit_filter_state_mediator_base& mediator)
{
	if (!ImGui::CollapsingHeader("Market data", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	if (ImGui::BeginCombo("API", lang::to_string(_selected_api))) {
		for (auto dst : {
			lang::data_source_type::none,
			lang::data_source_type::poe_ninja,
			lang::data_source_type::poe_watch
		}) {
			if (ImGui::Selectable(lang::to_string(dst), dst == _selected_api)) {
				_selected_api = dst;
				on_api_change(app, mediator);
			}

			if (dst == lang::data_source_type::none && ImGui::IsItemHovered()) {
				aux::on_hover_text_tooltip(
					"Do not use any market data.\n"
					"All autogenerations will output no items.\n"
					"Same behavior as if all items had no confidence.");
			}
		}

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("League", _selected_league.value_or("").c_str())) {
		const auto& available_leagues = app.available_leagues();

		for (const auto& league : available_leagues) {
			if (ImGui::Selectable(league.c_str(), league == _selected_league)) {
				_selected_league = league;
				on_league_change(app, mediator);
			}
		}

		ImGui::EndCombo();
	}
}

}
