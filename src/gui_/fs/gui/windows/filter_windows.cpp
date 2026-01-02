#include <fs/gui/windows/filter_windows_fwd.hpp>
#include <fs/gui/windows/filter_windows.hpp>

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

#ifndef __EMSCRIPTEN__
std::unique_ptr<filter_window> real_filter_window_from_file(std::string path)
{
	return std::make_unique<real_filter_window>(real_filter_window::from_file(std::move(path)));
}

std::unique_ptr<filter_window> spirit_filter_window_from_file(const network::cache& network_cache, std::string path)
{
	return std::make_unique<spirit_filter_window>(spirit_filter_window::from_file(network_cache, std::move(path)));
}
#endif

std::unique_ptr<filter_window> real_filter_window_from_text_input()
{
	return std::make_unique<real_filter_window>(real_filter_window::from_text_input());
}

std::unique_ptr<filter_window> spirit_filter_window_from_text_input(const network::cache& network_cache)
{
	return std::make_unique<spirit_filter_window>(spirit_filter_window::from_text_input(network_cache));
}

std::unique_ptr<filter_window> real_filter_window_from_source(std::string name, std::string source)
{
	return std::make_unique<real_filter_window>(real_filter_window::from_source(std::move(name), std::move(source)));
}

std::unique_ptr<filter_window> spirit_filter_window_from_source(const network::cache& network_cache, std::string name, std::string source)
{
	return std::make_unique<spirit_filter_window>(spirit_filter_window::from_source(network_cache, std::move(name), std::move(source)));
}

filter_window::filter_window()
: imgui_window({}, drawable_area_size()) {}

real_filter_window::real_filter_window()
{
	open();
}

#ifndef __EMSCRIPTEN__
real_filter_window real_filter_window::from_file(std::string path)
{
	real_filter_window window;
	window._state.load_source_file(std::move(path));
	return window;
}
#endif

real_filter_window real_filter_window::from_text_input()
{
	real_filter_window window;
	window._state.open_text_input();
	return window;
}

real_filter_window real_filter_window::from_source(std::string name, std::string source)
{
	real_filter_window window;
	window._state.new_source(std::move(source));
	window._state.source().name(std::move(name));
	return window;
}

void real_filter_window::draw_impl(
	const gui_settings& settings,
	const lang::loot::item_database& db,
	lang::loot::generator& gen,
	network::cache& network_cache)
{
	title(make_title(_state.source(), str_real_filter_from_text_input));
	_state.draw(settings, db, gen, network_cache);
}

spirit_filter_window::spirit_filter_window(const network::cache& network_cache)
: _state(network_cache.leagues.get_leagues())
{
	open();
}

#ifndef __EMSCRIPTEN__
spirit_filter_window spirit_filter_window::from_file(const network::cache& network_cache, std::string path)
{
	spirit_filter_window window(network_cache);
	window._state.load_source_file(std::move(path));
	return window;
}
#endif

spirit_filter_window spirit_filter_window::from_text_input(const network::cache& network_cache)
{
	spirit_filter_window window(network_cache);
	window._state.open_text_input();
	return window;
}

spirit_filter_window spirit_filter_window::from_source(const network::cache& network_cache, std::string name, std::string source)
{
	spirit_filter_window window(network_cache);
	window._state.new_source(std::move(source));
	window._state.source().name(std::move(name));
	return window;
}

void spirit_filter_window::draw_impl(
	const gui_settings& settings,
	const lang::loot::item_database& db,
	lang::loot::generator& gen,
	network::cache& network_cache)
{
	title(make_title(_state.source(), str_spirit_filter_from_text_input));
	_state.draw(settings, db, gen, network_cache);
}

}
