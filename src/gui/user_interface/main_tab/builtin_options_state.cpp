#include "user_interface/main_tab/builtin_options_state.hpp"
#include "ui_utils.hpp"

#include <elements/element.hpp>

namespace el = cycfi::elements;

void builtin_options_state::make_ui()
{
	_root_element = el::share(make_section("built-in generation options",
		el::no_hstretch(el::vtile(
			el::htile(
				el::vtile(
					el::align_left_middle(el::label("override opacity in all colors (0 (transparent) - 255 (opaque))")),
					el::align_left_middle(el::label("override volume in all built-in sounds (0 - 300)"))
				),
				el::vtile(
					el::hold(_override_opacity_option.ui()),
					el::hold(_override_volume_option.ui())
				)
			),
			el::htile(
				el::vtile(
					el::align_left_middle(el::label("API to download item prices from")),
					el::align_left_middle(el::label("league to download item prices for")),
					el::align_left_middle(el::label("item price data:"))
				),
				el::vtile(
					el::hold(_api_selection.ui()),
					el::hold(_league_selection.ui()),
					el::hold(_market_data.ui())
				)
			)
		))
	));
}
