#include "ui.hpp"

#include <elements/support.hpp>
#include <elements/element.hpp>

namespace
{

namespace el = cycfi::elements;

auto constexpr background_color = el::rgba(35, 35, 37, 255);

auto group_margin_size()
{
	return el::rect{ 10, 40, 10, 10 };
}

// ---- generation tab ----

auto make_tab_generate_input()
{
	auto path_options =
		el::group("path settings", el::margin(group_margin_size(),
			/*
			el::vtile(
				el::htile(
					el::left_caption(el::input_box().first, "path to filter template file"),
					el::button("browse...")
				),
				el::top_margin(10, el::htile(
					el::left_caption(el::input_box().first, "output directory"),
					el::button("browse..."))
				),
				el::top_margin(10, el::left_caption(el::input_box().first, "name scheme of generated files"))
			),
			*/
			// this is better:
			el::hmin_size(400, el::htile(
				el::vtile(
					el::align_middle(el::align_left(el::label("path to filter template file"))),
					el::align_middle(el::align_left(el::label("output directory"))),
					el::align_middle(el::align_left(el::label("name scheme of generated files")))
				),
				el::vtile(
					el::input_box().first,
					el::input_box().first,
					el::input_box().first
				),
				el::vtile(
					el::button("browse..."),
					el::button("browse..."),
					el::align_left(el::label(".filter"))
				)
			))
		));


	auto previously_generated_filtes = el::vtile(
		el::group("previously generated filtes", el::margin(group_margin_size(),
			el::min_size({150, 300}, el::box(el::colors::black)) // TODO
		)));

	return el::vtile(
		std::move(path_options),
		std::move(previously_generated_filtes),
		el::button("reload template and generate"));
}

auto make_tab_generate_options()
{
	auto price_data_downloads =
		el::group("price data downloads", el::margin(group_margin_size(), el::vtile(
			el::min_size({150, 300}, el::box(el::colors::black)) // TODO
		)));

	auto filter_generation_options =
		el::group("filter generation options", el::margin(group_margin_size(), el::vtile(
			el::min_size({150, 300}, el::box(el::colors::black)) // TODO
		)));

	return
		el::vtile(
			std::move(price_data_downloads),
			std::move(filter_generation_options)
		);
}

auto make_tab_generate_logs()
{
	return
		el::group("logs", el::margin(group_margin_size(), el::vtile(
			el::vscroller(el::min_size({150, 500}, el::box(el::colors::black)) /* <= replace with actual logs */),
			el::button("clear logs")
		)));
}

auto make_tab_generate()
{
	return
		el::margin(group_margin_size(),
			el::htile(
				el::align_top(el::margin({ 20, 20, 20, 20 }, make_tab_generate_input())),
				el::align_top(el::margin({ 20, 20, 20, 20 }, make_tab_generate_options())),
				el::align_top(el::margin({ 20, 20, 20, 20 }, make_tab_generate_logs()))
			)
		);
}

// ---- settings tab ----

auto make_tab_settings()
{
	auto proxy_settings =
		el::group("proxy", el::margin(group_margin_size(), el::vtile(

		)));

	auto security_settings =
		el::group("security", el::margin(group_margin_size(), el::vtile(

		)));

	auto other_settings =
		el::group("other", el::margin(group_margin_size(), el::vtile(

		)));

	auto networking_settings =
		el::group("networking settings", el::margin(group_margin_size(), el::vtile(
			std::move(proxy_settings),
			std::move(security_settings),
			std::move(other_settings)
		)));

	return networking_settings;
}

// ---- main ----

auto make_user_interface()
{
	// FIXME: temporary vscroller + vtile implementation
	// fix this when notebook widget is implemented
	return
		el::vscroller(el::vtile(
			make_tab_generate(),
			make_tab_settings()
		));
}

} // namespace

void make_and_bind_user_interface(cycfi::elements::view& view, user_state& /* state */)
{
	view.content(
		make_user_interface(),
		el::box(background_color)
	);
}
