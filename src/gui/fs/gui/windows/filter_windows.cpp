#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/windows/filter/filter_state.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

namespace fs::gui {

real_filter_window::real_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _state(app)
{
	open();
	_state.reload_source_file(real_filter_path());
}

void real_filter_window::draw_contents()
{
	_state.draw_interface(real_filter_path(), _application.get());
}

spirit_filter_window::spirit_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _state(app)
{
	open();
	_state.reload_source_file(spirit_filter_path());
}

void spirit_filter_window::draw_contents()
{
	_state.draw_interface(spirit_filter_path(), _application.get());
}

}
