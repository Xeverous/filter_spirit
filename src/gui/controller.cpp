#include "controller.hpp"

#include <fs/utility/visitor.hpp>

#include <variant>

namespace {

void process_event(events::load_item_database /* event */, user_interface& ui)
{
	ui.main().loot_preview().load_item_database(ui.main().logger().logger());
}

void process_event(events::filter_template_path_changed /* event */, user_interface& ui)
{
	ui.main().filter_template().load_filter_template(ui.main().logger().logger());
}

void process_event(events::filter_template_changed /* event */, user_interface& ui)
{
	// TODO grab settings from state instead of {}
	ui.main().filter_template().parse_filter_template({}, ui.main().logger().logger());
}

void recompute_real_filter(user_interface& ui)
{
	ui.main().filter_template().recompute_real_filter(
		ui.main().builtin_options().market_data().price_report(),
		ui.main().logger().logger());
}

void process_event(events::price_report_changed /* event */, user_interface& ui)
{
	recompute_real_filter(ui);
}

void process_event(events::spirit_filter_changed /* event */, user_interface& ui)
{
	recompute_real_filter(ui);
}

void refresh_loot_preview(user_interface& ui)
{
	const auto& maybe_real_filter = ui.main().filter_template().real_filter();
	if (maybe_real_filter)
		ui.main().loot_preview().refresh_loot_preview(*maybe_real_filter);
}

void process_event(events::real_filter_changed /* event */, user_interface& ui)
{
	refresh_loot_preview(ui);
}

void process_event(events::refresh_loot_preview /* event */, user_interface& ui)
{
	refresh_loot_preview(ui);
}

void refresh_market_data(user_interface& ui, bool force)
{
	ui.main().builtin_options().market_data().refresh_market_data(
		ui.main().builtin_options().api_selection().selected_api(),
		ui.main().builtin_options().league_selection().selected_league(),
		force ? boost::posix_time::seconds(0) : boost::posix_time::seconds(3600), // TODO grab from settings
		ui.settings().download_settings(),
		ui.main().logger().logger());
}

void process_event(events::league_selection_changed /* event */, user_interface& ui)
{
	refresh_market_data(ui, false);
}

void process_event(events::api_selection_changed /* event */, user_interface& ui)
{
	ui.main().builtin_options().league_selection().api_selection_changed(
		ui.main().builtin_options().api_selection().selected_api());

	refresh_market_data(ui, false);
}

void process_event(events::refresh_market_data /* event */, user_interface& ui)
{
	refresh_market_data(ui, true);
}

void process_event(events::refresh_available_leagues /* event */, user_interface& ui)
{
	ui.main().builtin_options().league_selection().refresh_available_leagues(
		ui.settings().download_settings(),
		ui.main().logger().logger());
}

void browse_for_filter_template(user_interface& ui, cycfi::elements::view& view)
{
	/*
	 * For some reason, if browsing (which uses modal window) is invoked
	 * directly, it keeps spawning infinitely many modal windows in a loop
	 * that seems to be outside of FS code. But if the function is posted
	 * through elements built-in ASIO queue, the bug does not happen. Perhaps
	 * it is because how/where elements implements OS-level event handling.
	 */
	view.post([&, window = view.host()]() {
		ui.main().filter_template().browse_for_filter_template(window);
	});
}

void process_event(event_t event, cycfi::elements::view& view, user_interface& ui)
{
	std::visit(fs::utility::visitor{
		[&](auto event) {
			process_event(std::move(event), ui);
		},
		[&](events::filter_template_browse_requested) {
			browse_for_filter_template(ui, view);
		}
	}, std::move(event));
}

} // namespace

void controller::process_events()
{
	while (!_event_queue.empty()) {
		process_event(std::move(_event_queue.front()), *this, _ui);
		_event_queue.pop();
	}
}
