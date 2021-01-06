#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/windows/filter/real_filter_state_mediator.hpp>
#include <fs/gui/windows/filter/spirit_filter_state_mediator.hpp>

#include <string>
#include <optional>
#include <functional>

namespace fs::gui {

class application;

class real_filter_window : public imgui_window
{
public:
#ifndef __EMSCRIPTEN__
	static real_filter_window from_file(application& app, std::string path);
#endif
	static real_filter_window from_text_input(application& app);
	static real_filter_window from_source(application& app, std::string name, std::string source);

protected:
	void draw_contents() override;

private:
	real_filter_window(application& app);

	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	real_filter_state_mediator _state;
};

class spirit_filter_window : public imgui_window
{
public:
#ifndef __EMSCRIPTEN__
	static spirit_filter_window from_file(application& app, std::string path);
#endif
	static spirit_filter_window from_text_input(application& app);
	static spirit_filter_window from_source(application& app, std::string name, std::string source);

protected:
	void draw_contents() override;

private:
	spirit_filter_window(application& app);

	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	spirit_filter_state_mediator _state;
};

}
