#include "user_interface.hpp"

#include <elements/support.hpp>
#include <elements/element.hpp>

namespace el = cycfi::elements;

namespace {

constexpr auto background_color = el::rgba(45, 45, 45, 255);

auto make_tab_reference()
{
	return el::label("TODO");
}

auto make_tab_about()
{
	return el::label("TODO");
}

template <typename E>
auto make_page(E&& element)
{
	return el::layer(el::align_left_top(std::forward<E>(element)), el::frame{});
}

} // namespace

user_interface::user_interface(el::host_window_handle window, el::view& v, event_inserter inserter)
: _main_tab(inserter)
{
	v.content(
		el::scale(1.2f, el::vnotebook(
			v,
			el::deck(
				make_page(el::hold(_main_tab.make_ui(window))),
				make_page(make_tab_reference()),
				make_page(el::hold(_settings_tab.make_ui())),
				make_page(make_tab_about())
			),
			el::tab("main"),
			el::tab("reference"),
			el::tab("settings"),
			el::tab("about")
		)),
		el::box(background_color)
	);
}
