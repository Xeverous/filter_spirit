#pragma once

#include <fs/gui/auxiliary/raii.hpp>

#include <imgui.h>

#include <string>
#include <string_view>
#include <utility>

namespace fs::gui {

class imgui_window
{
public:
	imgui_window(std::string title, std::string id)
	: _id(std::move(id))
	, _title(std::move(title))
	{
		recompute_window_str();
	}

	imgui_window(std::string title = {})
	: imgui_window(title, title)
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
		_title = str;
		recompute_window_str();
	}

	const std::string& name() const
	{
		return _title;
	}

	void draw()
	{
		if (!_is_opened)
			return;

		if (_force_focus) {
			ImGui::SetNextWindowFocus();
			_force_focus = false;
		}

		const auto _ = aux::scoped_pointer_id(this);

		if (ImGui::Begin(_imgui_window_str.c_str(), &_is_opened))
			draw_contents();

		ImGui::End();
	}

protected:
	virtual void draw_contents() = 0;

private:
	void recompute_window_str()
	{
		// "###" prevents Dear ImGui from considering full string as the window's ID
		// ...which allows to change displayed window title without losing any state
		_imgui_window_str = _title;
		_imgui_window_str.append("###");
		_imgui_window_str.append(_id);
	}

	std::string _id;
	std::string _title;
	std::string _imgui_window_str;
	bool _is_opened = false;
	bool _force_focus = false;
};

}
