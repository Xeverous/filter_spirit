#pragma once

#include <fs/utility/assert.hpp>

#include <imgui.h>

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

class fonting
{
public:
	fonting();

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

private:
	std::vector<font> _text_fonts;
	std::vector<font> _monospaced_fonts;
	std::size_t _selected_text_font_index = 0;
	std::size_t _selected_monospaced_font_index = 0;
	ImFont* _text_font = nullptr;
	ImFont* _monospaced_font = nullptr;
	int _text_font_size = 24;
	int _monospaced_font_size = 24;
	bool _monospaced_interface_font = false;

	bool _rebuild_needed = false;
	bool _update_needed = false;
};

}