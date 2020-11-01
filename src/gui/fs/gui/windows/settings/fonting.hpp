#pragma once

#include <fs/utility/assert.hpp>

#include <imgui.h>

#include <string>
#include <vector>

namespace fs::gui {

struct font
{
	std::string name;
	std::string font_data; // TTF or OTF file content
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
		return _text_fonts[_selected_text_font_index].name;
	}

	const std::string& selected_monospaced_font_name() const
	{
		FS_ASSERT(_selected_monospaced_font_index < _monospaced_fonts.size());
		return _monospaced_fonts[_selected_monospaced_font_index].name;
	}

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
	bool _rebuild_needed = false;
};

}
