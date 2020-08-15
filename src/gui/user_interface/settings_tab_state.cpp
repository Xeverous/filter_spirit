#include "user_interface/settings_tab_state.hpp"
#include "ui_utils.hpp"

#include <elements/element.hpp>

namespace el = cycfi::elements;

std::shared_ptr<el::element> settings_tab_state::make_ui()
{
	auto proxy_settings =
		make_section("proxy", el::htile(
			el::vtile(
				el::align_left_middle(el::label("proxy"))
			),
			el::vtile(
				el::input_box().first
			)
		));

	auto security_settings =
		make_section("security", el::vtile(
			el::check_box("SSL/TLS host verification"),
			el::check_box("SSL/TLS peer verification")
		));

	auto other_settings =
		make_section("other", el::htile(
			el::vtile(
				el::align_left_middle(el::label("timeout (ms)"))
			),
			el::vtile(
				el::input_box().first
			)
		));

	return
		el::share(make_section("networking settings", el::vtile(
			std::move(proxy_settings),
			std::move(security_settings),
			std::move(other_settings)
		)));
}
