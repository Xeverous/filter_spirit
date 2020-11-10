#pragma once

#include <fs/gui/ui_utils.hpp>

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

	bool is_opened() const noexcept { return _is_opened; }

	void open() { _is_opened = true; }
	void close() { _is_opened = false; }

	void take_focus()
	{
		_force_focus = true;
	}

	void name(std::string_view str)
	{
		_name = str;
	}

	const std::string& name() const
	{
		return _name;
	}

	void draw()
	{
		if (!_is_opened)
			return;

		if (_force_focus) {
			ImGui::SetNextWindowFocus();
			_force_focus = false;
		}

		const auto _ = scoped_pointer_id(this);

		if (ImGui::Begin(_name.c_str(), &_is_opened))
			draw_contents();

		ImGui::End();
	}

protected:
	virtual void draw_contents() = 0;

private:
	std::string _name;
	bool _is_opened = false;
	bool _force_focus = false;
};

}
