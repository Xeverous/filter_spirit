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
	imgui_window(std::string title, std::string id, ImVec2 size = {})
	: _id(std::move(id))
	, _title(std::move(title))
	, _size(size)
	{
		recompute_window_str();
	}

	imgui_window(std::string title = {}, ImVec2 size = {})
	: imgui_window(title, title, size)
	{
	}

	bool is_opened() const noexcept { return _is_opened; }

	void open() { _is_opened = true; }
	void close() { _is_opened = false; }

	void take_focus()
	{
		_force_focus = true;
	}

	void title(std::string_view str)
	{
		_title = str;
		recompute_window_str();
	}

	const std::string& title() const
	{
		return _title;
	}

protected:
	// function should draw contents
	template <typename F>
	void draw_window(F f)
	{
		if (!_is_opened)
			return;

		if (_force_focus) {
			ImGui::SetNextWindowFocus();
			_force_focus = false;
		}

		const auto _ = aux::scoped_pointer_id(this);

		ImGui::SetNextWindowSize(_size, ImGuiCond_FirstUseEver);
		if (ImGui::Begin(_imgui_window_str.c_str(), &_is_opened))
			f();

		ImGui::End();
	}

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
	ImVec2 _size;
};

}
