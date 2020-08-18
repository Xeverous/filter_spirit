#include "user_interface/main_tab_state.hpp"
#include "ui_utils.hpp"

#include <elements/element.hpp>

namespace el = cycfi::elements;

auto make_filter_supplied_options()
{
	return make_section("filter-supplied options", el::htile(
		el::align_center_middle(el::label("(upcoming feature)"))
	));
}

std::shared_ptr<el::element> main_tab_state::make_ui()
{
	return el::share(el::vtile(
		el::htile(
			el::no_hstretch(el::vtile(
				el::hold(_filter_template.ui()),
				el::hold(_builtin_options.ui()),
				make_filter_supplied_options()
			)),
			el::hold(_loot_preview.ui())
		),
		el::hold(_logger.ui())
	));
}
