#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

#include <utility>

namespace {

ImVec2 drawable_area_size()
{
	ImVec2 result = ImGui::GetIO().DisplaySize;
	result.y -= ImGui::GetFontSize();
	return result;
}

constexpr auto str_real_filter_text_input = "Real filter - from text input";
constexpr auto str_spirit_filter_text_input = "Spirit filter - from text input";

}

namespace fs::gui {

real_filter_window::real_filter_window(application& app, std::string path)
: imgui_window(std::move(path), drawable_area_size())
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.load_source_file(title());
}

real_filter_window::real_filter_window(application& app)
: imgui_window(str_real_filter_text_input, drawable_area_size())
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.open_text_input();
}

void real_filter_window::draw_contents()
{
	if (const auto* path = _state.source_path(); path)
		title(*path);
	else
		title(str_real_filter_text_input);

	_state.draw_interface(_application.get());
}

spirit_filter_window::spirit_filter_window(application& app, std::string path)
: imgui_window(std::move(path), drawable_area_size())
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.load_source_file(title());
}

spirit_filter_window::spirit_filter_window(application& app)
: imgui_window(str_real_filter_text_input, drawable_area_size())
, _application(std::ref(app))
, _state(app.font_settings())
{
	open();
	_state.open_text_input();
}

void spirit_filter_window::draw_contents()
{
	if (const auto* path = _state.source_path(); path)
		title(*path);
	else
		title(str_spirit_filter_text_input);

	_state.draw_interface(_application.get());
}

}
