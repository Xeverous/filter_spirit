#include "ui.hpp"
#include "user_state.hpp"
#include "platform/modal_dialogs.hpp"

#include <elements/support.hpp>
#include <elements/element.hpp>

#include <functional>

namespace
{

// ---- code for reuse ----

namespace el = cycfi::elements;

auto constexpr background_color = el::rgba(45, 45, 45, 255);

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

void reload_filter_template(user_state& state)
{
	state.load_filter_template();
	state.parse_filter_template();
}

auto make_path_settings(el::host_window_handle window, el::view& view, user_state& state)
{
	auto label_filter_template_path = el::share(el::label("(no filter template selected)"));

	auto on_filter_template_reload = [&state](bool) {
		reload_filter_template(state);
	};

	auto on_filter_template_browse = [label_filter_template_path, window, &view, &state](bool) mutable {
		filesystem_modal_settings settings;
		open_file_modal_settings of_settings;
		std::vector<std::string> paths = modal_dialog_open_file(window, settings, of_settings);

		if (paths.empty()) {
			label_filter_template_path->set_text("(no filter template selected)");
		}
		else {
			label_filter_template_path->set_text(paths.front());
			state.generation.filter_template_path = std::move(paths.front());
			reload_filter_template(state);
		}

		view.refresh();
	};

	return
		el::htile(
			el::align_center_middle(el::hold(label_filter_template_path)),
			make_button("browse...", on_filter_template_browse),
			make_button(el::icons::cycle, "reload", on_filter_template_reload)
		);
}

auto make_builtin_options_api_selection(user_state& state)
{
	constexpr auto poe_ninja = "poe.ninja";
	constexpr auto poe_watch = "poe.watch";
	constexpr auto none = "no API data";

	return
		el::vtile(el::htile(
			el::vtile(
				el::align_left_middle(el::label("item price data API")),
				el::align_left_middle(el::label("league to download item prices for"))
			),
			el::vtile(
				el::selection_menu(
					[&state](std::string_view selected)
					{
						if (selected == poe_ninja)
							state.generation.data_source = fs::lang::data_source_type::poe_ninja;
						else if (selected == poe_watch)
							state.generation.data_source = fs::lang::data_source_type::poe_watch;
						else
							state.generation.data_source = fs::lang::data_source_type::none;
					},
					{
						poe_ninja,
						poe_watch,
						none
					}
				).first,
				el::htile(
					el::selection_menu(
						[&state](std::string_view selected)
						{
							state.generation.league = selected;
						},
						{
							"Standard", "Hardcore"
						}
					).first,
					make_button(el::icons::cycle, "refresh", [](bool){})
				)
			)
		));
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

auto make_tab_main_logs(el::view& view, user_state& state)
{
	auto log_container = el::share(el::vtile_composite());
	state.logger.set_ui_elements(log_container, view);

	auto clear_logs = [log_container, &view](bool) mutable {
		log_container->clear();
		view.refresh();
	};

	return
		el::vtile(
			el::vscroller(
				el::min_size(
					{800, 150},
					el::layer(
						el::hold(std::move(log_container)),
						el::box(el::colors::black)
					)
				)
			),
			el::align_right(el::no_hstretch(el::htile(
				make_button(el::icons::docs, "copy logs", [](bool){}),
				make_button(el::icons::cancel, "clear logs", clear_logs)
			)))
		);
}

auto make_tab_main(el::host_window_handle window, el::view& view, user_state& state)
{
	return
		el::vtile(
			el::htile(
				el::vtile(
					make_path_settings(window, view, state),
					make_builtin_options(state),
					make_filter_supplied_options()
				),
				el::group("loot preview", el::align_center_middle(el::label("TODO").font_size(100)))
			),
			el::top_margin(3.0f, make_tab_main_logs(view, state))
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
	el::view& view,
	user_state& state)
{
	return el::vnotebook(
		view,
		el::deck(
			make_page(make_tab_main(window, view, state)),
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
	el::view& view,
	user_state& state)
{
	view.content(
		el::scale(1.2f, make_user_interface(window, view, state)),
		el::box(background_color)
	);
}
