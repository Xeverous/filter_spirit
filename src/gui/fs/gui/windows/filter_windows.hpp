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
	real_filter_window(application& app, std::string path);

protected:
	void draw_contents() override;

private:
	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	real_filter_state_mediator _state;
};

class spirit_filter_window : public imgui_window
{
public:
	spirit_filter_window(application& app, std::string path);

protected:
	void draw_contents() override;

private:
	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	spirit_filter_state_mediator _state;
};

}
