#include "ui.hpp"
#include "user_state.hpp"
#include "item_preview.hpp"
#include "platform/modal_dialogs.hpp"

#include <elements/support.hpp>
#include <elements/element.hpp>
#include <infra/support.hpp> // for remove_cvref_t

#include <fs/utility/async.hpp>
#include <fs/network/ggg/parse_data.hpp>

#include <functional>
#include <vector>
#include <string>
#include <string_view>
#include <chrono>

namespace
{

// ---- code for reuse ----

namespace el = cycfi::elements;

auto constexpr background_color = el::rgba(45, 45, 45, 255);

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

	if (result.empty())
		result = "0s";
	else
		result.pop_back(); // remove trailing space

	return result;
}

double calculate_progress(std::size_t done, std::size_t size)
{
	if (size == 0)
		return 0.0; // avoid division by 0, report no progress instead

	return static_cast<double>(done) / size;
}

auto make_progress_bar()
{
	return el::progress_bar(el::box(el::colors::black), el::box(el::get_theme().indicator_color));
}

auto make_refresh_button()
{
	return el::button(el::icons::cycle, "refresh");
}

template <typename Subject>
el::proxy<cycfi::remove_cvref_t<Subject>>
make_proxy(Subject&& subject)
{
	return { std::forward<Subject>(subject) };
}

auto group_margin_size()
{
	return el::rect{ 10, 40, 10, 10 };
}

auto make_button(std::string text, std::function<void(bool)> on_click)
{
	auto but = el::button(std::move(text));
	but.on_click = std::move(on_click);
	return but;
}

auto make_button(std::uint32_t icon_code, std::string text, std::function<void(bool)> on_click)
{
	auto but = el::button(icon_code, std::move(text));
	but.on_click = std::move(on_click);
	return but;
}

// ---- main tab ----

auto make_path_settings(el::host_window_handle window, user_state& state)
{
	auto label_filter_template_path = el::share(el::label("(no filter template selected)"));

	auto on_filter_template_reload = [&state](bool) {
		state.load_filter_template();
	};

	auto on_filter_template_browse = [label_filter_template_path, window, &state](bool) mutable {
		filesystem_modal_settings settings;
		open_file_modal_settings of_settings;
		std::vector<std::string> paths = modal_dialog_open_file(window, settings, of_settings);

		if (paths.empty()) {
			label_filter_template_path->set_text("(no filter template selected)");
		}
		else {
			label_filter_template_path->set_text(paths.front());
			state.generation.filter_template_path = std::move(paths.front());
			state.load_filter_template();
		}
	};

	return
		el::htile(
			el::align_center_middle(el::hold(label_filter_template_path)),
			make_button("browse...", on_filter_template_browse),
			make_button(el::icons::cycle, "reload", on_filter_template_reload)
		);
}

auto make_league_selection_menu(user_state& state)
{
	return el::selection_menu(
		[&state](std::string_view selected) {
			state.generation.league_state.selected_league = selected;
			state.generation.item_price_data_state.refresh_item_price_data(boost::posix_time::seconds(3600));
		},
		state.generation.league_state.available_leagues).first;
}

void update_download_state(
	el::deck_element& league_selection_refresh_element,
	el::progress_bar_base& league_selection_refresh_progress_bar,
	el::proxy<el::basic_menu>& league_selection_menu,
	user_state& state)
{
	if (!state.generation.league_state.download_running)
		return;

	if (fs::utility::is_ready(state.generation.league_state.leagues_future)) {
		state.generation.league_state.download_running = false;

		std::vector<fs::lang::league> leagues = fs::network::ggg::parse_league_info(
			state.generation.league_state.leagues_future.get());

		auto& available_leagues = state.generation.league_state.available_leagues;
		available_leagues.clear();
		for (const auto& league : leagues)
			available_leagues.push_back(std::move(league.name));

		league_selection_menu.subject(make_league_selection_menu(state));
		league_selection_refresh_element.select(1); // swicth to refresh button
		state.logger.logger().info() << available_leagues.size() << " non-SSF leagues are available\n";
	}
	else {
		const auto xfer_info = state.generation.league_state.league_download_info.xfer_info.load(std::memory_order_relaxed);
		league_selection_refresh_progress_bar.value(
			calculate_progress(xfer_info.bytes_downloaded_so_far, xfer_info.expected_download_size));
	}
}

void update_item_price_data_state(
	el::deck_element& item_price_data_refresh_element,
	el::progress_bar_base& item_price_data_refresh_progress_bar,
	el::label& item_price_data_status_label,
	user_state& state)
{
	if (state.generation.item_price_data_state.download_running) {
		if (fs::utility::is_ready(state.generation.item_price_data_state.data_future)) {
			state.generation.item_price_data_state.download_running = false;
			state.generation.item_price_data_state.price_report = state.generation.item_price_data_state.data_future.get();
			state.recompute_real_filter();

			if (state.generation.item_price_data_state.price_report.metadata.data_source == fs::lang::data_source_type::none)
				item_price_data_refresh_element.select(2); // switch to no-refresh label
			else
				item_price_data_refresh_element.select(1); // switch to refresh button
		}
		else {
			auto& download_info = state.generation.item_price_data_state.download_info;
			const auto requests_complete = download_info.requests_complete.load(std::memory_order_relaxed);
			const auto requests_total    = download_info.requests_total.load(std::memory_order_relaxed);
			item_price_data_refresh_progress_bar.value(calculate_progress(requests_complete, requests_total));
		}
	}
	else {
		// no update running - just update UI label
		const auto& download_date = state.generation.item_price_data_state.price_report.metadata.download_date;

		if (!download_date.is_special()) {
			const auto now = boost::posix_time::microsec_clock::universal_time();
			const auto time_diff = now - download_date;
			item_price_data_status_label.set_text("using cached data from " + to_string(time_diff) + " ago");
		}
	}
}

auto make_builtin_options_api_selection(user_state& state)
{
	// -- league selection --

	auto league_selection_menu = el::share(make_proxy(make_league_selection_menu(state)));

	auto league_selection_refresh_progress_bar = el::share(make_progress_bar());
	auto league_selection_refresh_button = el::share(make_refresh_button());

	auto league_selection_refresh_element = el::share(el::deck(
		el::hold(league_selection_refresh_progress_bar),
		el::hold(league_selection_refresh_button)
	));
	league_selection_refresh_element->select(1);

	state.generation.league_state.refresh_available_leagues = [=, &state]() mutable {
		if (state.generation.league_state.download_running)
			return; // ignore button click if download is already in progress

		league_selection_refresh_element->select(0); // switch to progress bar
		league_selection_refresh_progress_bar->value(0);

		state.generation.league_state.leagues_future = fs::network::ggg::async_download_leagues(
			state.program.networking, &state.generation.league_state.league_download_info, state.logger.logger());
		state.generation.league_state.download_running = true;
	};
	league_selection_refresh_button->on_click = [&state](bool) {
		state.generation.league_state.refresh_available_leagues();
	};

	state.generation.league_state.update = [=, &state]() {
		update_download_state(
			*league_selection_refresh_element,
			*league_selection_refresh_progress_bar,
			*league_selection_menu,
			state);
	};

	// -- item price data management --

	auto item_price_data_status_label = el::share(el::label(""));
	auto item_price_data_refresh_progress_bar = el::share(make_progress_bar());
	auto item_price_data_refresh_button = el::share(make_refresh_button());

	auto item_price_data_refresh_element = el::share(el::deck(
		el::hold(item_price_data_refresh_progress_bar),
		el::hold(item_price_data_refresh_button),
		el::label("(nothing to refresh)")
	));
	item_price_data_refresh_element->select(1);

	state.generation.item_price_data_state.refresh_item_price_data = [=, &state](boost::posix_time::time_duration expiration_time) mutable {
		if (state.generation.league_state.selected_league.empty())
			return;

		item_price_data_refresh_element->select(0); // switch to progress bar
		item_price_data_refresh_progress_bar->value(0);

		state.generation.item_price_data_state.data_future = state.generation.item_price_data_state.cache.async_get_report(
			state.generation.league_state.selected_league,
			state.generation.item_price_data_state.selected_api,
			expiration_time,
			state.program.networking,
			&state.generation.item_price_data_state.download_info,
			state.logger.logger());
		state.generation.item_price_data_state.download_running = true;
		item_price_data_status_label->set_text("downloading...");
	};
	item_price_data_refresh_button->on_click = [&state](bool) {
		state.generation.item_price_data_state.refresh_item_price_data(boost::posix_time::seconds(0));
	};

	state.generation.item_price_data_state.update = [=, &state]() {
		update_item_price_data_state(
			*item_price_data_refresh_element,
			*item_price_data_refresh_progress_bar,
			*item_price_data_status_label,
			state);
	};

	///

	constexpr auto poe_ninja = "poe.ninja";
	constexpr auto poe_watch = "poe.watch";
	constexpr auto none = "none";

	return
		el::htile(
			el::vtile(
				el::align_left_middle(el::label("API to download item prices from")),
				el::align_left_middle(el::label("league to download item prices for")),
				el::align_left_middle(el::label("item price data:"))
			),
			el::vtile(
				el::selection_menu(
					[item_price_data_refresh_element, &state](std::string_view selected) {
						if (selected == poe_ninja) {
							state.generation.item_price_data_state.selected_api = fs::lang::data_source_type::poe_ninja;
						}
						else if (selected == poe_watch) {
							state.generation.item_price_data_state.selected_api = fs::lang::data_source_type::poe_watch;
						}
						else {
							state.generation.item_price_data_state.selected_api = fs::lang::data_source_type::none;
							item_price_data_refresh_element->select(3); // switch to the label
						}
					},
					{ poe_ninja, poe_watch, none }
				).first,
				el::htile(
					el::hold(league_selection_menu),
					el::hold(league_selection_refresh_element)
				),
				el::htile(
					el::hold(item_price_data_status_label),
					el::hold(item_price_data_refresh_element)
				)
			)
		);
}

auto make_builtin_options(user_state& state)
{
	return
		el::group("built-in generation options", el::margin(group_margin_size(),
			el::vtile(
				el::htile(
					el::vtile(
						el::align_left_middle(el::label("override opacity in all colors (0 (transparent) - 255 (opaque))")),
						el::align_left_middle(el::label("override volume in all built-in sounds (0 - 300)"))
					),
					el::no_hstretch(el::vtile(
						el::input_box().first,
						el::input_box().first
					))
				),
				make_builtin_options_api_selection(state)
			)
		));
}

auto make_filter_supplied_options()
{
	return
		el::group("filter-supplied options", el::margin(group_margin_size(), el::htile(
			el::align_center_middle(el::label("(upcoming feature)"))
		)));
}

auto make_tab_main_logs(user_state& state)
{
	auto clear_logs = [&state](bool) mutable {
		state.logger.clear_logs();
	};

	return
		el::vtile(
			el::layer(
				el::hold(state.logger.make_ui()),
				el::box(el::colors::black)
			),
			el::htile(
				make_button(el::icons::docs, "copy logs", [](bool){}),
				make_button(el::icons::cancel, "clear logs", clear_logs)
			)
		);
}

auto make_tab_main_loot_preview()
{
	static auto flow_composite = el::flow_composite{};
	flow_composite.push_back(share(item_preview("item preview", el::color(255, 0, 0), el::color(255, 0, 0), el::color(0, 255, 0))));

	return el::group("loot preview", el::margin(group_margin_size(),
		el::htile(
			el::no_hstretch(el::vtile(el::button("generate loot"))),
			el::align_top(el::flow(flow_composite))
		)
	));
}

auto make_tab_main(el::host_window_handle window, user_state& state)
{
	return
		el::vtile(
			el::htile(
				el::vtile(
					make_path_settings(window, state),
					make_builtin_options(state),
					make_filter_supplied_options()
				),
				make_tab_main_loot_preview()
			),
			el::top_margin(3.0f, make_tab_main_logs(state))
		);
}

// ---- reference tab ----

auto make_tab_reference()
{
	return el::label("TODO");
}

// ---- settings tab ----

auto make_tab_settings()
{
	auto proxy_settings =
		el::group("proxy", el::margin(group_margin_size(), el::htile(
			el::vtile(
				el::align_left_middle(el::label("proxy"))
			),
			el::vtile(
				el::input_box().first
			)
		)));

	auto security_settings =
		el::group("security", el::margin(group_margin_size(), el::vtile(
			el::check_box("SSL/TLS host verification"),
			el::check_box("SSL/TLS peer verification")
		)));

	auto other_settings =
		el::group("other", el::margin(group_margin_size(), el::htile(
			el::vtile(
				el::align_left_middle(el::label("timeout (ms)"))
			),
			el::vtile(
				el::input_box().first
			)
		)));

	auto networking_settings =
		el::group("networking settings", el::margin(group_margin_size(), el::vtile(
			std::move(proxy_settings),
			std::move(security_settings),
			std::move(other_settings)
		)));

	return networking_settings;
}

auto make_tab_about()
{
	return el::label("TODO");
}

// ---- enclosing code ----

template <typename E>
auto make_page(E&& element)
{
	return el::layer(el::align_left_top(std::forward<E>(element)), el::frame{});
}

auto make_tab(std::string text)
{
	return el::tab(std::move(text));
}

auto
make_user_interface(
	el::host_window_handle window,
	el::view& v,
	user_state& state)
{
	return el::vnotebook(
		v,
		el::deck(
			make_page(make_tab_main(window, state)),
			make_page(make_tab_reference()),
			make_page(make_tab_settings()),
			make_page(make_tab_about())
		),
		make_tab("main"),
		make_tab("reference"),
		make_tab("settings"),
		make_tab("about"));
}

} // namespace

void
make_and_bind_user_interface(
	el::host_window_handle window,
	el::view& v,
	user_state& state)
{
	v.content(
		el::scale(1.2f, make_user_interface(window, v, state)),
		el::box(background_color)
	);
}
