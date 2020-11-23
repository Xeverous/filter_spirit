#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

#include <utility>

namespace {

constexpr auto str_real_filter_text_input = "Real filter - from text input";
constexpr auto str_spirit_filter_text_input = "Spirit filter - from text input";

}

namespace fs::gui {

real_filter_window::real_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.load_source_file(name());
}

real_filter_window::real_filter_window(application& app)
: imgui_window(str_real_filter_text_input)
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.open_text_input();
}

void real_filter_window::draw_contents()
{
	if (const auto& path = _state.source_path(); path.has_value())
		name(*path);
	else
		name(str_real_filter_text_input);

	_state.draw_interface(_application.get());
}

spirit_filter_window::spirit_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.load_source_file(name());
}

spirit_filter_window::spirit_filter_window(application& app)
: imgui_window(str_real_filter_text_input)
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.open_text_input();
}

void spirit_filter_window::draw_contents()
{
	if (const auto& path = _state.source_path(); path.has_value())
		name(*path);
	else
		name(str_spirit_filter_text_input);

	_state.draw_interface(_application.get());
}

}
