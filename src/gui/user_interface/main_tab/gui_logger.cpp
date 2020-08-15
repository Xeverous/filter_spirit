#include "user_interface/main_tab/gui_logger.hpp"
#include "ui_utils.hpp"

#include <fs/utility/assert.hpp>

#include <elements/support.hpp>

namespace el = cycfi::elements;

namespace {

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

std::shared_ptr<el::element> gui_logger::make_ui()
{
	_composite = el::share(el::vtile_composite());
	auto scroller = el::share(el::vscroller(el::hold(_composite)));

	_scroll_to_bottom = [scroller]() {
		scroller->valign(1.0);
	};

	return el::share(el::top_margin(3.0f, el::vtile(
		el::layer(
			el::hold(scroller),
			el::box(el::colors::black)
		),
		el::htile(
			make_button(el::icons::docs, "copy logs", [](bool){}),
			make_button(el::icons::cancel, "clear logs", [this](bool) mutable {
				clear_logs();
			})
		)
	)));
}

void gui_logger::update()
{
	BOOST_ASSERT(_composite != nullptr);
	BOOST_ASSERT(_scroll_to_bottom);

	_logger([&](fs::log::buffer_logger& logger) {
		for (const fs::log::buffer_logger::message& m : logger.messages()) {
			_composite->push_back(el::share(
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

	BOOST_ASSERT(_composite != nullptr);
	_composite->clear();
}
