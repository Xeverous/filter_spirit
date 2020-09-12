#include "user_interface/generic/slider.hpp"

#include <utility>

namespace el = cycfi::elements;

slider::slider(double init_value, std::function<void(double)> on_change)
{
	make_ui(init_value, std::move(on_change));
}

void slider::make_ui(double init_value, std::function<void(double)> on_change)
{
	auto slider = el::slider(
		el::fixed_size({20, 20}, el::rbox(el::colors::medium_blue, 5)),
		el::vsize(10, el::box(el::colors::black)),
		init_value
	);
	slider.on_change = std::move(on_change);

	_root_element = el::share(el::vmargin({3, 3}, std::move(slider)));
}
