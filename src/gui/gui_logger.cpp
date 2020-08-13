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

std::string remove_trailing_whitespace(std::string str)
{
	auto const is_whitespace = [](char c) {
		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
	};

	while (!str.empty() && is_whitespace(str.back()))
		str.pop_back();

	return str;
}

}

void gui_logger::update_ui()
{
	BOOST_ASSERT(_ui_element != nullptr);
	BOOST_ASSERT(_scroll_to_bottom);

	_logger([&](fs::log::buffer_logger& logger) {
		for (const fs::log::buffer_logger::message& m : logger.messages()) {
			_ui_element->push_back(el::share(
				el::no_vstretch(el::align_left(el::static_text_box(
					remove_trailing_whitespace(m.text),
					el::get_theme().text_box_font,
					el::get_theme().text_box_font_size,
					severity_to_color(m.s)
				)))
			));
		}

		// scroll only when new logs appear
		if (!logger.messages().empty())
			_scroll_to_bottom();

		logger.clear();
	});
}

void gui_logger::clear_logs()
{
	_logger([&](fs::log::buffer_logger& logger) {
		logger.clear();
	});
	_ui_element->clear();
}
