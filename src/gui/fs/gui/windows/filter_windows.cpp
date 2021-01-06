#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

#include <utility>

namespace {

using namespace fs::gui;

ImVec2 drawable_area_size()
{
	ImVec2 result = ImGui::GetIO().DisplaySize;
	result.y -= ImGui::GetFontSize();
	return result;
}

constexpr auto str_real_filter_text_input = "Real filter - from text input";
constexpr auto str_spirit_filter_text_input = "Spirit filter - from text input";

std::string make_title(const source_state& state, const char* text_input_title)
{
	if (const auto* path = state.path(); path) {
		if (state.is_source_edited())
			return *path + " [edited]";
		else
			return *path;
	}
	else if (const auto* name = state.name(); name) {
		if (state.is_source_edited())
			return *name + " [edited]";
		else
			return *name;
	}
	else {
		return text_input_title;
	}
}

}

namespace fs::gui {

real_filter_window::real_filter_window(application& app)
: imgui_window({}, drawable_area_size())
, _application(std::ref(app))
{
	open();
}

#ifndef __EMSCRIPTEN__
real_filter_window real_filter_window::from_file(application& app, std::string path)
{
	real_filter_window window(app);
	window._state.load_source_file(path);
	return window;
}
#endif

real_filter_window real_filter_window::from_text_input(application& app)
{
	real_filter_window window(app);
	window._state.open_text_input();
	return window;
}

real_filter_window real_filter_window::from_source(application& app, std::string name, std::string source)
{
	real_filter_window window(app);
	window._state.new_source(std::move(source));
	window._state.source().name(std::move(name));
	return window;
}

void real_filter_window::draw_contents()
{
	title(make_title(_state.source(), str_real_filter_text_input));
	_state.draw_interface(_application.get());
}

spirit_filter_window::spirit_filter_window(application& app)
: imgui_window({}, drawable_area_size())
, _application(std::ref(app))
, _state(app.leagues_cache().get_leagues())
{
	open();
}

#ifndef __EMSCRIPTEN__
spirit_filter_window spirit_filter_window::from_file(application& app, std::string path)
{
	spirit_filter_window window(app);
	window._state.load_source_file(path);
	return window;
}
#endif

spirit_filter_window spirit_filter_window::from_text_input(application& app)
{
	spirit_filter_window window(app);
	window._state.open_text_input();
	return window;
}

spirit_filter_window spirit_filter_window::from_source(application& app, std::string name, std::string source)
{
	spirit_filter_window window(app);
	window._state.new_source(std::move(source));
	window._state.source().name(std::move(name));
	return window;
}

void spirit_filter_window::draw_contents()
{
	title(make_title(_state.source(), str_spirit_filter_text_input));
	_state.draw_interface(_application.get());
}

}
