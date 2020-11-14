#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/windows/filter/filter_state.hpp>

#include <string>
#include <utility>
#include <optional>
#include <functional>

namespace fs::gui {

class application;

class real_filter_window : public imgui_window
{
public:
	real_filter_window(application& app, std::string path);

	const std::string& real_filter_path() const
	{
		return name();
	}

protected:
	void draw_contents() override;

private:
	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	real_filter_state _state;
};

class spirit_filter_window : public imgui_window
{
public:
	spirit_filter_window(application& app, std::string path);

	const std::string& spirit_filter_path() const
	{
		return name();
	}

protected:
	void draw_contents() override;

private:
	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	spirit_filter_state _state;
};

}
