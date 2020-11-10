#pragma once

#include <fs/gui/imgui_window.hpp>
#include <fs/gui/filter_state.hpp>
#include <fs/gui/gui_logger.hpp>

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
	// use string instead of string_view because filesystem wants C-strings
	void reload_real_filter_file(const std::string& path);

	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	gui_logger _logger;
	std::string _status;
	std::optional<real_filter_state> _state; // empty if loading file failed
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
	// use string instead of string_view because filesystem wants C-strings
	void reload_spirit_filter_file(const std::string& path);

	std::reference_wrapper<application> _application; // plain reference type would make this class non-moveable
	gui_logger _logger;
	std::string _status;
	std::optional<spirit_filter_state> _state; // empty if loading file failed
};

}
