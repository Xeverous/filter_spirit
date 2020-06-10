#include "gui_logger.hpp"

#include <fs/utility/assert.hpp>

#include <elements/support.hpp>
#include <elements/element/label.hpp>
#include <elements/element/align.hpp>

namespace el = cycfi::elements;

namespace
{

el::color severity_to_color(fs::log::severity s)
{
	if (s == fs::log::severity::info)
		return el::colors::white;
	else if (s == fs::log::severity::warning)
		return el::colors::orange;
	else if (s == fs::log::severity::error)
		return el::colors::red;

	return el::colors::white;
}

void remove_trailing_whitespace(std::string& str)
{
	auto const is_whitespace = [](char c) {
		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
	};

	while (!str.empty() && is_whitespace(str.back()))
		str.pop_back();
}

}

void gui_logger::end_message()
{
	BOOST_ASSERT(_ui_element != nullptr);
	BOOST_ASSERT(_view != nullptr);

	remove_trailing_whitespace(_string_logger.str());

	_ui_element->push_back(el::share(
		el::align_left(el::static_text_box(
			std::move(_string_logger.str()),
			el::get_theme().text_box_font,
			el::get_theme().text_box_font_size,
			severity_to_color(_severity)
		))
	));
	_string_logger.str().clear();

	_view->refresh();
}
