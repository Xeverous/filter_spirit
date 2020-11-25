#pragma once

#include <imgui.h>

#include <optional>
#include <functional>
#include <vector>

namespace fs::parser {

class lookup_data;
class line_lookup;

}

namespace fs::lang {

class item;
class item_filtering_result;

}

namespace fs::gui {

class fonting;

struct debug_info
{
	std::reference_wrapper<const parser::lookup_data> lookup;
	std::reference_wrapper<const parser::line_lookup> lines;
	std::reference_wrapper<const lang::item> itm;
	std::reference_wrapper<const lang::item_filtering_result> result;
};

class debug_state
{
public:
	void open_debug(
		const parser::lookup_data& lookup,
		const parser::line_lookup& lines,
		const lang::item& itm,
		const lang::item_filtering_result& result)
	{
		_info = debug_info{std::ref(lookup), std::ref(lines), std::ref(itm), std::ref(result)};
		recompute();
		open_popup();
	}

	void open_popup()
	{
		_popup_pending = true;
	}

	void close_debug()
	{
		invalidate();
		close_popup();
	}

	void close_popup()
	{
		_popup_open = false;
	}

	void invalidate()
	{
		_info = std::nullopt;
	}

	void recompute();

	void draw_interface(const fonting& f);

private:
	// popup state
	bool _popup_pending = false;
	bool _popup_open = false;

	// debug state
	std::optional<debug_info> _info;
	std::vector<ImU32> _line_colors;
	float _drawing_offset_y = 0.0f;
};

}
