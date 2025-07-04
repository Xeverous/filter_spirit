#pragma once

#include <fs/gui/auxiliary/raii.hpp>
#include <fs/lang/constants.hpp>
#include <fs/utility/assert.hpp>

#include <imgui.h>

#include <array>
#include <string>
#include <vector>
#include <utility>

namespace fs::gui {

class font
{
public:
	font(std::string path, std::string name)
	: _name(std::move(name))
	, _path(std::move(path))
	{
	}

	const std::string& name() const
	{
		return _name;
	}

	const std::string& path() const
	{
		return _path;
	}

	const std::string& data()
	{
		if (!is_loaded())
			load();

		return _font_data;
	}

	bool is_loaded() const
	{
		return !_font_data.empty();
	}

	void load();

private:
	std::string _name;
	std::string _path;
	std::string _font_data; // TTF or OTF file content
};

constexpr int num_item_preview_fonts()
{
	// +1 because both range ends are inclusive
	return lang::constants::max_filter_font_size - lang::constants::min_filter_font_size + 1;
}

class font_settings
{
public:
	font_settings();

	void build_default_fonts();

	void draw_font_selection_ui();

	const std::string& selected_text_font_name() const
	{
		FS_ASSERT(_selected_text_font_index < _text_fonts.size());
		return _text_fonts[_selected_text_font_index].name();
	}

	const std::string& selected_monospaced_font_name() const
	{
		FS_ASSERT(_selected_monospaced_font_index < _monospaced_fonts.size());
		return _monospaced_fonts[_selected_monospaced_font_index].name();
	}

	void update();
	void rebuild();
	bool is_rebuild_needed() const
	{
		return _rebuild_needed;
	}

	auto text_font() const
	{
		FS_ASSERT(_text_font != nullptr);
		return _text_font;
	}

	int text_font_size() const
	{
		return _text_font_size;
	}

	int monospaced_font_size() const
	{
		return _monospaced_font_size;
	}

	auto monospaced_font() const
	{
		FS_ASSERT(_monospaced_font != nullptr);
		return _monospaced_font;
	}

	aux::scoped_font_override scoped_monospaced_font() const
	{
		return { monospaced_font() };
	}

	ImFont* filter_preview_font(int size) const;

private:
	std::vector<font> _text_fonts;
	std::vector<font> _monospaced_fonts;
	std::size_t _selected_text_font_index = 0;
	std::size_t _selected_monospaced_font_index = 0;
	ImFont* _text_font = nullptr;
	ImFont* _monospaced_font = nullptr;
	std::array<ImFont*, num_item_preview_fonts()> _filter_preview_fonts = {};
	int _text_font_size = 24;
	int _monospaced_font_size = 24;
	bool _monospaced_interface_font = false;

	bool _rebuild_needed = false;
	bool _update_needed = false;
};

}
