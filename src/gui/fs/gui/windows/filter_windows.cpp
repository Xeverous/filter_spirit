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
	else {
		return text_input_title;
	}
}

}

namespace fs::gui {

real_filter_window::real_filter_window(application& app, std::string path)
: imgui_window(std::move(path), drawable_area_size())
, _application(std::ref(app))
{
	open();
	_state.load_source_file(title());
}

real_filter_window::real_filter_window(application& app)
: imgui_window(str_real_filter_text_input, drawable_area_size())
, _application(std::ref(app))
{
	open();
	_state.open_text_input();
}

void real_filter_window::draw_contents()
{
	title(make_title(_state.source(), str_real_filter_text_input));
	_state.draw_interface(_application.get());
}

spirit_filter_window::spirit_filter_window(application& app, std::string path)
: imgui_window(std::move(path), drawable_area_size())
, _application(std::ref(app))
, _state(app.leagues_cache().get_leagues())
{
	open();
	_state.load_source_file(title());
}

spirit_filter_window::spirit_filter_window(application& app)
: imgui_window(str_real_filter_text_input, drawable_area_size())
, _application(std::ref(app))
, _state(app.leagues_cache().get_leagues())
{
	open();
	_state.open_text_input();
}

void spirit_filter_window::draw_contents()
{
	title(make_title(_state.source(), str_spirit_filter_text_input));
	_state.draw_interface(_application.get());
}

}
