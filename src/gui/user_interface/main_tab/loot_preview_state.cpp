#include "user_interface/main_tab/loot_preview_state.hpp"
#include "item_preview.hpp"
#include "ui_utils.hpp"

namespace el = cycfi::elements;

std::shared_ptr<cycfi::elements::element> loot_preview_state::make_ui()
{
	_flow_composite = el::flow_composite{};
	_flow_composite.push_back(el::share(item_preview("item preview", el::color(255, 0, 0), el::color(255, 0, 0), el::color(0, 255, 0))));

	return el::share(make_section("loot preview",
		el::htile(
			el::no_hstretch(el::vtile(el::button("generate loot"))),
			el::align_top(el::flow(_flow_composite))
		)
	));
}
