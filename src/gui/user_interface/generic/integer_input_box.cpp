#include "user_interface/generic/integer_input_box.hpp"

#include <elements/element.hpp>

namespace el = cycfi::elements;

std::shared_ptr<el::element> integer_input_box::make_ui()
{
	return el::share(el::input_box().first);
}
