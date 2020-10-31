#pragma once

#include <imgui.h>

#include <string>
#include <string_view>
#include <utility>

namespace fs::gui {

class imgui_window
{
public:
	imgui_window(std::string name = {})
	: _name(std::move(name))
	{
	}

	virtual ~imgui_window() = default;

	bool is_visible() const noexcept { return _show; }

	void show() { _show = true; }
	void hide() { _show = false; }

	void take_focus()
	{
		_force_focus = true;
	}

	void name(std::string_view str)
	{
		_name = str;
	}

	const char* name() const
	{
		return _name.c_str();
	}

	void draw()
	{
		if (!_show)
			return;

		if (_force_focus) {
			ImGui::SetNextWindowFocus();
			_force_focus = false;
		}

		if (!ImGui::Begin(_name.c_str(), &_show)) {
			ImGui::End();
			return;
		}

		draw_contents();

		ImGui::End();
	}

protected:
	virtual void draw_contents() = 0;

private:
	std::string _name;
	bool _show = false;
	bool _force_focus = false;
};

}
