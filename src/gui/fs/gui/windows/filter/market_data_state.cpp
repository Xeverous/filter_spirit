#include <fs/gui/windows/filter/market_data_state.hpp>
#include <fs/gui/windows/filter/spirit_filter_state_mediator.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/application.hpp>
#include <fs/network/ggg/download_data.hpp>
#include <fs/network/ggg/parse_data.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/async.hpp>

#include <imgui.h>

#include <array>
#include <initializer_list>
#include <cstdio>
#include <exception>

namespace {

std::string to_string(boost::posix_time::time_duration duration)
{
	std::string result;

	if (auto d = duration.hours() / 24; d != 0)
		result.append(std::to_string(d)).append("d ");

	if (auto h = duration.hours() % 24; h != 0)
		result.append(std::to_string(h)).append("h ");

	if (auto m = duration.minutes(); m != 0)
		result.append(std::to_string(m)).append("m ");

	if (auto s = duration.seconds(); s != 0)
		result.append(std::to_string(s)).append("s ");

	if (!result.empty())
		result.pop_back(); // remove trailing space

	return result;
}

float calculate_progress(std::size_t done, std::size_t size)
{
	if (size == 0)
		return 0.0f; // avoid division by 0, report no progress instead

	return static_cast<float>(done) / size;
}

}

namespace fs::gui {

void market_data_state::refresh_item_price_report(application& app, spirit_filter_state_mediator& mediator)
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

	if (_price_report_download_running)
		return;

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

void market_data_state::refresh_available_leagues(application& app, spirit_filter_state_mediator& mediator)
{
	if (_leagues_download_running)
		return;

	if (!_leagues_download_info)
		_leagues_download_info = std::make_shared<network::download_info>();

	_leagues_future = std::async(
		std::launch::async, [
			settings = app.network_settings().download_settings(),
			download_info = _price_report_download_info,
			logger = mediator.share_logger()]()
		{
			network::ggg::api_league_data api_data =
				network::ggg::download_leagues(settings, download_info.get(), *logger);

			network::update_leagues_on_disk(api_data, *logger);
			return network::ggg::parse_league_info(api_data);
		}
	);
	_leagues_download_running = true;
}

void market_data_state::check_downloads(application& app, spirit_filter_state_mediator& mediator)
{
	if (_leagues_download_running) {
		if (utility::is_ready(_leagues_future)) {
			try {
				_available_leagues = _leagues_future.get();
				app.leagues_cache().set_leagues(_available_leagues);
				mediator.logger().info() << "League data download complete.\n";
			}
			catch (const std::exception& e) {
				mediator.logger().error() << "League download failed: " << e.what() << "\n";
			}
			_leagues_download_running = false;
			on_league_change(app, mediator);
		}
	}

	if (_price_report_download_running) {
		if (utility::is_ready(_price_report_future)) {
			try {
				_price_report = _price_report_future.get();
				mediator.logger().info() << "Market data download complete.\n";
			}
			catch (const std::exception& e) {
				mediator.logger().error() << "Market data download failed: " << e.what() << "\n";
			}
			_price_report_download_running = false;
			refresh_item_price_report(app, mediator);
		}
	}
}

void market_data_state::draw_interface(application& app, spirit_filter_state_mediator& mediator)
{
	check_downloads(app, mediator);

	if (!ImGui::CollapsingHeader("Market data", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	if (ImGui::BeginCombo("API", lang::to_string(_selected_api))) {
		const auto make_selectable = [&, this](lang::data_source_type dst, ImGuiSelectableFlags flags = 0) {
			if (ImGui::Selectable(lang::to_string(dst), _selected_api == dst, flags))
			{
				_selected_api = dst;
				on_api_change(app, mediator);
			}
		};

		make_selectable(lang::data_source_type::none);
		aux::on_hover_text_tooltip(
			"Do not use any market data.\n"
			"All autogenerations will output no items.\n"
			"Same behavior as if all items had no confidence.");

		make_selectable(lang::data_source_type::poe_ninja);

		make_selectable(lang::data_source_type::poe_watch, ImGuiSelectableFlags_Disabled);
		aux::on_hover_text_tooltip("Under construction. Website's API recently got reborn and refactored.");

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("League", _selected_league.value_or("").c_str())) {
		for (const auto& league : _available_leagues) {
			if (ImGui::Selectable(league.name.c_str(), league.name == _selected_league)) {
				_selected_league = league.name;
				on_league_change(app, mediator);
			}
		}

		ImGui::EndCombo();
	}

	ImGui::SameLine();

	if (_leagues_download_running) {
		ImGui::Text("downloading...");
	}
	else {
		if (ImGui::Button("Refresh"))
			refresh_available_leagues(app, mediator);
	}

	ImGui::Text("Status: ");
	ImGui::SameLine(0, 0); // (0, 0) to merge text

	if (_price_report_download_running) {
		ImGui::Text("downloading...");
		FS_ASSERT(_price_report_download_info != nullptr);
		std::array<char, 32> buf;
		ImGui::SameLine();

		const std::size_t requests_complete = _price_report_download_info->requests_complete.load(std::memory_order::memory_order_relaxed);
		const std::size_t requests_total    = _price_report_download_info->requests_total   .load(std::memory_order::memory_order_relaxed);
		const std::size_t bytes_downloaded  = _price_report_download_info->xfer_info.load(std::memory_order::memory_order_relaxed).bytes_downloaded_so_far;
		std::snprintf(buf.data(), buf.size(), "%zu/%zu (%zu bytes)", requests_complete, requests_total, bytes_downloaded);
		ImGui::ProgressBar(calculate_progress(requests_complete, requests_total), ImVec2(-1.0f, 0.0f), buf.data());
	}
	else {
		if (auto source = _price_report.metadata.data_source; source == lang::data_source_type::none) {
			ImGui::Text("using no market data");
		}
		else {
			const auto now = boost::posix_time::microsec_clock::universal_time();
			const auto time_diff = now - _price_report.metadata.download_date;
			ImGui::Text(
				"using cached market data from %s from %s ago",
				lang::to_string(source),
				to_string(time_diff).c_str());
		}
	}
}

}
