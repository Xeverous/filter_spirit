#include "user_interface/main_tab_state.hpp"
#include "item_preview.hpp"
#include "ui_utils.hpp"

#include <elements/element.hpp>

namespace el = cycfi::elements;

auto make_filter_supplied_options()
{
	return make_section("filter-supplied options", el::htile(
		el::align_center_middle(el::label("(upcoming feature)"))
	));
}

auto make_tab_main_loot_preview()
{
	static auto flow_composite = el::flow_composite{};
	flow_composite.push_back(el::share(item_preview("item preview", el::color(255, 0, 0), el::color(255, 0, 0), el::color(0, 255, 0))));

	return make_section("loot preview",
		el::htile(
			el::no_hstretch(el::vtile(el::button("generate loot"))),
			el::align_top(el::flow(flow_composite))
		)
	);
}

std::shared_ptr<el::element> main_tab_state::make_ui(el::host_window_handle window)
{
	return el::share(el::vtile(
		el::htile(
			el::vtile(
				el::hold(_filter_template.make_ui(window)),
				el::hold(_builtin_options.make_ui()),
				make_filter_supplied_options()
			),
			make_tab_main_loot_preview()
		),
		el::hold(_logger.make_ui())
	));
}
