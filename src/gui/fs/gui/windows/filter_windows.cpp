#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

#include <utility>

namespace fs::gui {

real_filter_window::real_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.load_source_file(name());
}

void real_filter_window::draw_contents()
{
	if (const auto& path = _state.source_path(); path.has_value())
		name(*path);
	else
		name("Real filter - from text input");

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

void spirit_filter_window::draw_contents()
{
	if (const auto& path = _state.source_path(); path.has_value())
		name(*path);
	else
		name("Spirit filter - from text input");

	_state.draw_interface(_application.get());
}

}
